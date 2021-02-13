/*
 *   Copyright (C) 2021  Daniel Vrátil <dvratil@kde.org>
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

#include "providerbase.h"
#include "klipperinterface.h"
#include "plasmapass_debug.h"

#include <QClipboard>
#include <QCryptographicHash>
#include <QGuiApplication>
#include <QMimeData>
#include <QProcess>
#include <QStandardPaths>

#include <QDBusConnection>

#include <Plasma/DataEngine>
#include <Plasma/DataEngineConsumer>
#include <Plasma/PluginLoader>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

#include <KLocalizedString>

#include <chrono>
#include <utility>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace
{
constexpr const auto DefaultSecretTimeout = 45s;
constexpr const auto SecretTimeoutUpdateInterval = 100ms;

const QString klipperDBusService = QStringLiteral("org.kde.klipper");
const QString klipperDBusPath = QStringLiteral("/klipper");
const QString klipperDataEngine = QStringLiteral("org.kde.plasma.clipboard");

}

using namespace PlasmaPass;

ProviderBase::ProviderBase(const QString &path, QObject *parent)
    : QObject(parent)
    , mSecretTimeout(DefaultSecretTimeout)
{
    mTimer.setInterval(SecretTimeoutUpdateInterval);
    connect(&mTimer, &QTimer::timeout, this, [this]() {
        mTimeout -= mTimer.interval();
        Q_EMIT timeoutChanged();
        if (mTimeout == 0) {
            expireSecret();
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

    QStringList args = {QStringLiteral("-d"),
                        QStringLiteral("--quiet"),
                        QStringLiteral("--yes"),
                        QStringLiteral("--compress-algo=none"),
                        QStringLiteral("--no-encrypt-to"),
                        path};
    if (isGpg2) {
        args = QStringList{QStringLiteral("--batch"), QStringLiteral("--use-agent")} + args;
    }

    mGpg = std::make_unique<QProcess>();
    // Let's not behave like animals and deal with this asynchronously
    connect(mGpg.get(), &QProcess::errorOccurred, this, [this, gpgExe](QProcess::ProcessError state) {
        if (state == QProcess::FailedToStart) {
            qCWarning(PLASMAPASS_LOG, "Failed to start %s: %s", qUtf8Printable(gpgExe), qUtf8Printable(mGpg->errorString()));
            setError(i18n("Failed to decrypt password: Failed to start GPG"));
        }
    });
    connect(mGpg.get(), &QProcess::readyReadStandardOutput, this, [this]() {
        while (!mGpg->atEnd()) {
            const auto line = QString::fromUtf8(mGpg->readLine()).trimmed();
            if (handleSecret(line) == HandlingResult::Stop) {
                break;
            }
        }
    });
    connect(mGpg.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this]() {
        const auto err = mGpg->readAllStandardError();
        if (mSecret.isEmpty()) {
            if (err.isEmpty()) {
                setError(i18n("Failed to decrypt password"));
            } else {
                setError(i18n("Failed to decrypt password: %1", QString::fromUtf8(err)));
            }
        }

        mGpg.reset();
    });
    mGpg->setProgram(gpgExe);
    mGpg->setArguments(args);
    mGpg->start(QIODevice::ReadOnly);
}

ProviderBase::~ProviderBase()
{
    if (mGpg) {
        mGpg->terminate();
    }
}

bool ProviderBase::isValid() const
{
    return !mSecret.isNull();
}

QString ProviderBase::secret() const
{
    return mSecret;
}

namespace {

QMimeData *mimeDataForPassword(const QString &password)
{
    auto mimeData = new QMimeData;
    mimeData->setText(password);
    // https://phabricator.kde.org/D12539
    mimeData->setData(QStringLiteral("x-kde-passwordManagerHint"), "secret");
    return mimeData;
}

} // namespace

void ProviderBase::setSecret(const QString &secret)
{
    auto clipboard = qGuiApp->clipboard(); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    clipboard->setMimeData(mimeDataForPassword(secret), QClipboard::Clipboard);

    if (clipboard->supportsSelection()) {
        clipboard->setMimeData(mimeDataForPassword(secret), QClipboard::Selection);
    }

    mSecret = secret;
    Q_EMIT validChanged();
    Q_EMIT secretChanged();

    mTimeout = defaultTimeout();
    Q_EMIT timeoutChanged();
    mTimer.start();
}

void ProviderBase::setSecretTimeout(std::chrono::seconds timeout)
{
    mSecretTimeout = timeout;
}

void ProviderBase::expireSecret()
{
    removePasswordFromClipboard(mSecret);

    mSecret.clear();
    mTimer.stop();
    Q_EMIT validChanged();
    Q_EMIT secretChanged();

    // Delete the provider, it's no longer needed
    deleteLater();
}

int ProviderBase::timeout() const
{
    return mTimeout;
}

int ProviderBase::defaultTimeout() const
{
    return duration_cast<milliseconds>(mSecretTimeout).count();
}

QString ProviderBase::error() const
{
    return mError;
}

bool ProviderBase::hasError() const
{
    return !mError.isNull();
}

void ProviderBase::setError(const QString &error)
{
    mError = error;
    Q_EMIT errorChanged();
}

void ProviderBase::removePasswordFromClipboard(const QString &password)
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
    const auto service = engine->serviceForSource(QString::fromLatin1(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha1).toBase64()));
    if (service == nullptr) {
        qCWarning(PLASMAPASS_LOG, "Failed to obtain PlasmaService for the password, falling back to clearClipboard()");
        mEngineConsumer.reset();
        clearClipboard();
        return;
    }

    auto job = service->startOperationCall(service->operationDescription(QStringLiteral("remove")));

    // FIXME: KJob::result() is an overloaded QPrivateSignal and cannot be QOverload()ed,
    // so we have to do it the old-school way
    connect(job, &KJob::result, this, &ProviderBase::onPlasmaServiceRemovePasswordResult);
}

void ProviderBase::onPlasmaServiceRemovePasswordResult(KJob *job)
{
    // Disconnect from the job: Klipper's ClipboardJob is buggy and emits result() twice
    disconnect(job, &KJob::result, this, &ProviderBase::onPlasmaServiceRemovePasswordResult);
    QTimer::singleShot(0, this, [this]() {
        mEngineConsumer.reset();
    });

    auto serviceJob = qobject_cast<Plasma::ServiceJob *>(job);
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

void ProviderBase::clearClipboard()
{
    org::kde::klipper::klipper klipper(klipperDBusService, klipperDBusPath, QDBusConnection::sessionBus());
    if (!klipper.isValid()) {
        return;
    }

    klipper.clearClipboardHistory();
    klipper.clearClipboardContents();
}
