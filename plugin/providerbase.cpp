// Copyright (C) 2021  Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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

#include <QGpgME/DecryptJob>
#include <QGpgME/Protocol>
#include <gpgme++/decryptionresult.h>

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
    , mPath(path)
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

    QTimer::singleShot(0, this, &ProviderBase::start);
}

ProviderBase::~ProviderBase() = default;

void ProviderBase::start()
{
    QFile file(mPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(PLASMAPASS_LOG, "Failed to open password file: %s", qUtf8Printable(file.errorString()));
        setError(i18n("Failed to open password file: %s", file.errorString()));
        return;
    }

    auto decryptJob = QGpgME::openpgp()->decryptJob();
    connect(decryptJob, &QGpgME::DecryptJob::result, this, [this](const GpgME::DecryptionResult &result, const QByteArray &plainText) {
        if (result.error()) {
            qCWarning(PLASMAPASS_LOG, "Failed to decrypt password: %s", result.error().asString());
            setError(i18n("Failed to decrypt password: %s", QString::fromUtf8(result.error().asString())));
            return;
        }

        const auto data = QString::fromUtf8(plainText);
        const auto lines = data.splitRef(QLatin1Char('\n'));
        for (const auto &line : lines) {
            if (handleSecret(line) == HandlingResult::Stop) {
                break;
            }
        }
    });

    const auto error = decryptJob->start(file.readAll());
    if (error) {
        qCWarning(PLASMAPASS_LOG, "Failed to decrypt password: %s", error.asString());
        setError(i18n("Failed to decrypt password: %s", QString::fromUtf8(error.asString())));
        return;
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

    qCDebug(PLASMAPASS_LOG, "Successfully removed password from Klipper");
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
