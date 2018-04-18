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

#include <QProcess>
#include <QStandardPaths>
#include <QDebug>

#define PASSWORD_TIMEOUT 45

using namespace PlasmaPass;

PasswordProvider::PasswordProvider(const QString &path, QObject *parent)
    : QObject(parent)
{
    mTimer.setInterval(1000);
    connect(&mTimer, &QTimer::timeout,
            this, [this]() {
                --mTimeout;
                Q_EMIT timeoutChanged();
                if (mTimeout == 0) {
                    mTimer.stop();
                }
            });

    bool isGpg2 = true;
    auto gpgExe = QStandardPaths::findExecutable(QStringLiteral("gpg2"));
    if (gpgExe.isEmpty()) {
        gpgExe = QStandardPaths::findExecutable(QStringLiteral("gpg"));
        isGpg2 = false;
    }
    if (gpgExe.isEmpty()) {
        // TODO: Error handling!
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
    connect(mGpg, &QProcess::stateChanged,
            this, [this](QProcess::ProcessState state) {
                if (state == QProcess::NotRunning) {
                    // TODO: Error handling!
                }
            });
    connect(mGpg, &QProcess::readyReadStandardOutput,
            this, [this]() {
                // We only read the first line, second line usually the username
                mPassword = QString::fromUtf8(mGpg->readLine()).trimmed();
                Q_EMIT validChanged();
                Q_EMIT passwordChanged();
                Q_EMIT timeoutChanged();
                mTimeout = PASSWORD_TIMEOUT;
                mTimer.start();
            });
    connect(mGpg, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this]() {
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

int PasswordProvider::timeout() const
{
    return mTimeout;
}
