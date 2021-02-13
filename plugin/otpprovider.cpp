/*
 *   Copyright (C) 2021 Daniel Vrátil <dvratil@kde.org>
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

#include "otpprovider.h"

#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QDateTime>

#include <KLocalizedString>

#include <chrono>

#include <liboath/oath.h>

using namespace PlasmaPass;
using namespace std::chrono_literals;

namespace {

static const QString otpAuthSchema = QStringLiteral("otpauth://");
static const QString secretQueryItem = QStringLiteral("secret");

QString parseOtpType(const QUrl &url)
{
    return url.host();
}

} // namespace

OTPProvider::OTPProvider(const QString &path, QObject *parent)
    : ProviderBase(path, parent)
{
    setSecretTimeout(30s);
}

ProviderBase::HandlingResult OTPProvider::handleSecret(const QString &secret)
{
    if (!secret.startsWith(otpAuthSchema)) {
        return HandlingResult::Continue;
    }

    QUrl url(secret);
    const auto otpType = parseOtpType(url);
    if (otpType == QLatin1String("totp")) {
        handleTOTP(url);
    } else {
        setError(i18n("Unsupported OTP type %1", otpType));
        return HandlingResult::Stop;
    }

    return HandlingResult::Stop;
}


void OTPProvider::handleTOTP(const QUrl &url)
{
    const QUrlQuery query(url.query());
    const auto secret = query.queryItemValue(secretQueryItem).toUtf8();

    char *decodedSecret = {};
    size_t decodedSecretLen = 0;
    oath_base32_decode(secret.data(), secret.size(), &decodedSecret, &decodedSecretLen);

    char output_otp[6] = {};
    oath_totp_generate(decodedSecret, decodedSecretLen,
            QDateTime::currentDateTime().toSecsSinceEpoch(),
            OATH_TOTP_DEFAULT_TIME_STEP_SIZE,
            OATH_TOTP_DEFAULT_START_TIME,
            6,
            output_otp);

    setSecret(QString::fromLatin1(output_otp, sizeof(output_otp)));
}



