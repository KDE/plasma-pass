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

#ifndef OTPPROVIDER_H_
#define OTPPROVIDER_H_

#include "providerbase.h"

namespace PlasmaPass
{
class PasswordsModel;

class OTPProvider : public ProviderBase
{
    Q_OBJECT

    friend class PasswordsModel;
protected:
    explicit OTPProvider(const QString &path, QObject *parent = nullptr);

    HandlingResult handleSecret(const QString &secret) override;

private:
    void handleTOTP(const QUrl &url);
};

}

#endif // OTPPROVIDER_H_
