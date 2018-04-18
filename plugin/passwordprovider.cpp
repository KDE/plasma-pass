/*
 *   Copyright (C) 2018  Daniel Vrátil <dvratil@kde.org>
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "passwordprovider.h"
#include "klipperinterface.h"
#include "plasmapass_debug.h"

#include <QProcess>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QClipboard>
#include <QGuiApplication>

#include <QDBusConnection>

#include <Plasma/PluginLoader>
#include <Plasma/DataEngineConsumer>
#include <Plasma/DataEngine>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

#include <KLocalizedString>

#include <chrono>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace {

static const auto PasswordTimeout = 45s;
static const auto PasswordTimeoutUpdateInterval = 100ms;

}

#define KLIPPER_DBUS_SERVICE QStringLiteral("org.kde.klipper")
#define KLIPPER_DBUS_PATH QStringLiteral("/klipper")
#define KLIPPER_DATA_ENGINE QStringLiteral("org.kde.plasma.clipboard")

using namespace PlasmaPass;

PasswordProvider::PasswordProvider(const QString &path, QObject *parent)
    : QObject(parent)
{
    mTimer.setInterval(duration_cast<milliseconds>(PasswordTimeoutUpdateInterval).count());
    connect(&mTimer, &QTimer::timeout,
            this, [this]() {
                mTimeout -= mTimer.interval();
                Q_EMIT timeoutChanged();
                if (mTimeout == 0) {
                    expirePassword();
                }
            });

    bool isGpg2 = true;
    auto gpgExe = QStandardPaths::findExecutable(QStringLiteral("gpg2"));
    if (gpgExe.isEmpty()) {
        gpgExe = QStandardPaths::findExecutable(QStringLiteral("gpg"));
        isGpg2 = false;
    }
    if (gpgExe.isEmpty()) {
        qCWarning(PLASMAPASS_LOG, "Failed to find gpg or gpg2 executables");
        setError(i18n("Failed to decrypt password: GPG is not available"));
        return;
    }

    QStringList args = { QStringLiteral("-d"),
                         QStringLiteral("--quiet"),
                         QStringLiteral("--yes"),
                         QStringLiteral("--compress-algo=none"),
                         QStringLiteral("--no-encrypt-to"),
                         path };
    if (isGpg2) {
        args = QStringList{ QStringLiteral("--batch"), QStringLiteral("--use-agent") } + args;
    }

    mGpg = new QProcess;
    // Let's not be like animals and deal with this asynchronously
    connect(mGpg, &QProcess::errorOccurred,
            this, [this, gpgExe](QProcess::ProcessError state) {
                if (state == QProcess::FailedToStart) {
                    qCWarning(PLASMAPASS_LOG, "Failed to start %s: %s", qUtf8Printable(gpgExe), qUtf8Printable(mGpg->errorString()));
                    setError(i18n("Failed to decrypt password: Failed to start GPG"));
                }
            });
    connect(mGpg, &QProcess::readyReadStandardOutput,
            this, [this]() {
                // We only read the first line, second line usually the username
                setPassword(QString::fromUtf8(mGpg->readLine()).trimmed());
            });
    connect(mGpg, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this]() {
                if (mPassword.isEmpty()) {
                    setError(i18n("Failed to decrypt password"));
                }

                mGpg->deleteLater();
                mGpg = nullptr;
            });
    mGpg->setProgram(gpgExe);
    mGpg->setArguments(args);
    mGpg->start(QIODevice::ReadOnly);
}

PasswordProvider::~PasswordProvider()
{
    if (mGpg) {
        mGpg->terminate();
        delete mGpg;
    }
}

bool PasswordProvider::isValid() const
{
    return !mPassword.isNull();
}

QString PasswordProvider::password() const
{
    return mPassword;
}

void PasswordProvider::setPassword(const QString &password)
{
    qGuiApp->clipboard()->setText(password);
    mPassword = password;
    Q_EMIT validChanged();
    Q_EMIT passwordChanged();

    mTimeout = defaultTimeout();
    Q_EMIT timeoutChanged();
    mTimer.start();
}

void PasswordProvider::expirePassword()
{
    removePasswordFromClipboard(mPassword);

    mPassword.clear();
    mTimer.stop();
    Q_EMIT validChanged();
    Q_EMIT passwordChanged();
}

int PasswordProvider::timeout() const
{
    return mTimeout;
}

int PasswordProvider::defaultTimeout() const
{
    return duration_cast<milliseconds>(PasswordTimeout).count();
}

QString PasswordProvider::error() const
{
    return mError;
}

bool PasswordProvider::hasError() const
{
    return !mError.isNull();
}

void PasswordProvider::setError(const QString &error)
{
    mError = error;
    Q_EMIT errorChanged();
}


void PasswordProvider::removePasswordFromClipboard(const QString &password)
{
    // Clear the WS clipboard itself
    const auto clipboard = qGuiApp->clipboard();
    if (clipboard->text() == password) {
        clipboard->clear();
    }

    const auto engine = Plasma::DataEngineConsumer().dataEngine(KLIPPER_DATA_ENGINE);

    // Klipper internally identifies each history entry by it's SHA1 hash
    // (see klipper/historystringitem.cpp) so we try here to obtain a service directly
    // for the history item with our password so that we can only remove the
    // password from the history without having to clear the entire history.
    const auto service = engine->serviceForSource(
        QString::fromLatin1(
            QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha1).toBase64()));
    if (!service) {
        qCWarning(PLASMAPASS_LOG, "Failed to obtain PlasmaService for the password, falling back to clearClipboard()");
        clearClipboard();
        return;
    }

    auto job = service->startOperationCall(service->operationDescription(QStringLiteral("remove")));

    // FIXME: KJob::result() is an overloaded QPrivateSignal and cannot be QOverload()ed,
    // so we have to do it the old-school way
    connect(job, SIGNAL(result(KJob*)), this, SLOT(onPlasmaServiceRemovePasswordResult(KJob*)));
}

void PasswordProvider::onPlasmaServiceRemovePasswordResult(KJob* job)
{
    // Disconnect from the job: Klipper's ClipboardJob is buggy and emits result() twice
    disconnect(job, SIGNAL(result(KJob*)), this, SLOT(onPlasmaServiceRemovePasswordResult(KJob*)));

    auto serviceJob = qobject_cast<Plasma::ServiceJob*>(job);
    if (serviceJob->error()) {
        qCWarning(PLASMAPASS_LOG, "ServiceJob for clipboard failed: %s", qUtf8Printable(serviceJob->errorString()));
        clearClipboard();
        return;
    }
    // If something went wrong fallback to clearing the entire clipboard
    if (!serviceJob->result().toBool()) {
        qCWarning(PLASMAPASS_LOG, "ServiceJob for clipboard failed internally, falling back to clearClipboard()");
        clearClipboard();
        return;
    }

    qCDebug(PLASMAPASS_LOG, "Successfuly removed password from Klipper");
}

void PasswordProvider::clearClipboard()
{
    org::kde::klipper::klipper klipper(KLIPPER_DBUS_SERVICE, KLIPPER_DBUS_PATH, QDBusConnection::sessionBus());
    if (!klipper.isValid()) {
        return;
    }

    klipper.clearClipboardHistory();
    klipper.clearClipboardContents();
}
