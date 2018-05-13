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

#include "plasmapassplugin.h"
#include "passwordsmodel.h"
#include "passwordsortproxymodel.h"
#include "passwordfiltermodel.h"
#include "passwordprovider.h"

#include <QJSEngine>
#include <QQmlEngine>
#include <QQmlContext>

void PlasmaPassPlugin::registerTypes(const char* uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.private.plasmapass"));

    qmlRegisterType<PlasmaPass::PasswordsModel>(uri, 1, 0, "PasswordsModel");
    qmlRegisterType<PlasmaPass::PasswordSortProxyModel>(uri, 1, 0, "PasswordSortProxyModel");
    qmlRegisterType<PlasmaPass::PasswordFilterModel>(uri, 1, 0, "PasswordFilterModel");
    qmlRegisterUncreatableType<PlasmaPass::PasswordProvider>(uri, 1, 0, "PasswordProvider", "");

    qmlProtectModule("org.kde.plasma.private.plasmapass", 1);
}
