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

#include "passwordsortproxymodel.h"
#include "passwordsmodel.h"

#include <QDebug>

using namespace PlasmaPass;

PasswordSortProxyModel::PasswordSortProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    sort(0); // enable sorting
}

bool PasswordSortProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const auto typeLeft = static_cast<PasswordsModel::EntryType>(source_left.data(PasswordsModel::EntryTypeRole).toInt());
    const auto typeRight = static_cast<PasswordsModel::EntryType>(source_right.data(PasswordsModel::EntryTypeRole).toInt());

    // Folders first
    if (typeLeft != typeRight) {
        return typeLeft == PasswordsModel::FolderEntry;
    }

    return QSortFilterProxyModel::lessThan(source_left, source_right);
}
