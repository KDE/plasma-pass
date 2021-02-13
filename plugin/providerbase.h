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

#ifndef PROVIDERBASE_H_
#define PROVIDERBASE_H_

#include <QObject>
#include <QTimer>

#include <memory>

class QProcess;
class QDBusPendingCallWatcher;
class KJob;
class QMimeData;

namespace Plasma
{
class DataEngineConsumer;
}

namespace PlasmaPass
{
class PasswordsModel;

class ProviderBase : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
    Q_PROPERTY(int timeout READ timeout NOTIFY timeoutChanged)
    Q_PROPERTY(int defaultTimeout READ defaultTimeout CONSTANT)
    Q_PROPERTY(QString secret READ secret NOTIFY secretChanged)
    Q_PROPERTY(bool hasError READ hasError NOTIFY errorChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

    friend class PasswordsModel;
public:
    ~ProviderBase() override;

    QString secret() const;
    bool isValid() const;
    int timeout() const;
    int defaultTimeout() const; // in milliseconds
    bool hasError() const;
    QString error() const;

Q_SIGNALS:
    void secretChanged();
    void validChanged();
    void timeoutChanged();
    void errorChanged();

protected:
    explicit ProviderBase(const QString &path, QObject *parent = nullptr);

    void setSecret(const QString &secret);
    void setSecretTimeout(std::chrono::seconds timeout);
    void setError(const QString &error);

    enum class HandlingResult {
        Continue,
        Stop
    };
    virtual HandlingResult handleSecret(const QString &secret) = 0;

private Q_SLOTS:
    void onPlasmaServiceRemovePasswordResult(KJob *job);

private:
    void expireSecret();

    void removePasswordFromClipboard(const QString &password);
    static void clearClipboard();

    std::unique_ptr<Plasma::DataEngineConsumer> mEngineConsumer;
    std::unique_ptr<QProcess> mGpg;
    QString mPath;
    QString mError;
    QString mSecret;
    QTimer mTimer;
    int mTimeout = 0;
    std::chrono::seconds mSecretTimeout;
};

}
#endif
