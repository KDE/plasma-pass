/*
 *   Copyright (C) 2018  Daniel Vrátil <dvratil@kde.org>
 *
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
#include <QMimeData>

#include <QDBusConnection>

#include <Plasma/PluginLoader>
#include <Plasma/DataEngineConsumer>
#include <Plasma/DataEngine>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

#include <KLocalizedString>

#include <chrono>
#include <utility>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace {

constexpr const auto PasswordTimeout = 45s;
constexpr const auto PasswordTimeoutUpdateInterval = 100ms;

const QString klipperDBusService = QStringLiteral("org.kde.klipper");
const QString klipperDBusPath = QStringLiteral("/klipper");
const QString klipperDataEngine = QStringLiteral("org.kde.plasma.clipboard");

}

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

    mGpg = std::make_unique<QProcess>();
    // Let's not be like animals and deal with this asynchronously
    connect(mGpg.get(), &QProcess::errorOccurred,
            this, [this, gpgExe](QProcess::ProcessError state) {
                if (state == QProcess::FailedToStart) {
                    qCWarning(PLASMAPASS_LOG, "Failed to start %s: %s", qUtf8Printable(gpgExe), qUtf8Printable(mGpg->errorString()));
                    setError(i18n("Failed to decrypt password: Failed to start GPG"));
                }
            });
    connect(mGpg.get(), &QProcess::readyReadStandardOutput,
            this, [this]() {
                // We only read the first line, second line usually the username
                setPassword(QString::fromUtf8(mGpg->readLine()).trimmed());
            });
    connect(mGpg.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this]() {
                const auto err = mGpg->readAllStandardError();
                if (mPassword.isEmpty()) {
                    if (err.isEmpty()) {
                        setError(i18n("Failed to decrypt password"));
                    } else {
                        setError(i18n("Failed to decrypt password: %1").arg(QString::fromUtf8(err)));
                    }
                }

                mGpg.reset();
            });
    mGpg->setProgram(gpgExe);
    mGpg->setArguments(args);
    mGpg->start(QIODevice::ReadOnly);
}

PasswordProvider::~PasswordProvider()
{
    if (mGpg) {
        mGpg->terminate();
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

QMimeData *PasswordProvider::mimeDataForPassword(const QString &password)
{
    auto mimeData = new QMimeData;
    mimeData->setText(password);
    // https://phabricator.kde.org/D12539
    mimeData->setData(QStringLiteral("x-kde-passwordManagerHint"), "secret");
    return mimeData;
}

void PasswordProvider::setPassword(const QString &password)
{
    auto clipboard = qGuiApp->clipboard(); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    clipboard->setMimeData(mimeDataForPassword(password), QClipboard::Clipboard);

    if (clipboard->supportsSelection()) {
        clipboard->setMimeData(mimeDataForPassword(password), QClipboard::Selection);
    }

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

    // Delete the provider, it's no longer needed
    deleteLater();
}

int PasswordProvider::timeout() const
{
    return mTimeout;
}

int PasswordProvider::defaultTimeout() const // NOLINT(readability-convert-member-functions-to-static)
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
    const auto clipboard = qGuiApp->clipboard(); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    if (clipboard->text() == password) {
        clipboard->clear();
    }

    if (!mEngineConsumer) {
        mEngineConsumer = std::make_unique<Plasma::DataEngineConsumer>();
    }
    auto engine = mEngineConsumer->dataEngine(klipperDataEngine);

    // Klipper internally identifies each history entry by it's SHA1 hash
    // (see klipper/historystringitem.cpp) so we try here to obtain a service directly
    // for the history item with our password so that we can only remove the
    // password from the history without having to clear the entire history.
    const auto service = engine->serviceForSource(
        QString::fromLatin1(
            QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha1).toBase64()));
    if (service == nullptr) {
        qCWarning(PLASMAPASS_LOG, "Failed to obtain PlasmaService for the password, falling back to clearClipboard()");
        mEngineConsumer.reset();
        clearClipboard();
        return;
    }

    auto job = service->startOperationCall(service->operationDescription(QStringLiteral("remove")));

    // FIXME: KJob::result() is an overloaded QPrivateSignal and cannot be QOverload()ed,
    // so we have to do it the old-school way
    connect(job, &KJob::result, this, &PasswordProvider::onPlasmaServiceRemovePasswordResult);
}

void PasswordProvider::onPlasmaServiceRemovePasswordResult(KJob* job)
{
    // Disconnect from the job: Klipper's ClipboardJob is buggy and emits result() twice
    disconnect(job, &KJob::result, this, &PasswordProvider::onPlasmaServiceRemovePasswordResult);
    QTimer::singleShot(0, this, [this]() { mEngineConsumer.reset(); });

    auto serviceJob = qobject_cast<Plasma::ServiceJob*>(job);
    if (serviceJob->error() != 0) {
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
    org::kde::klipper::klipper klipper(klipperDBusService, klipperDBusPath, QDBusConnection::sessionBus());
    if (!klipper.isValid()) {
        return;
    }

    klipper.clearClipboardHistory();
    klipper.clearClipboardContents();
}
