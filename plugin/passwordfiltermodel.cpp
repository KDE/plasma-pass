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

#include "passwordfiltermodel.h"
#include "passwordsmodel.h"
#include "abbreviations.h"

#include <KDescendantsProxyModel>

#include <QDebug>

using namespace PlasmaPass;

PasswordFilterModel::PasswordFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , mFlatModel(new KDescendantsProxyModel(this))
{
    mFlatModel->setDisplayAncestorData(false);
    sort(0); // enable sorting
}

void PasswordFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    mFlatModel->setSourceModel(sourceModel);

    if (!this->sourceModel()) {
        QSortFilterProxyModel::setSourceModel(mFlatModel);
    }
}

QString PasswordFilterModel::passwordFilter() const
{
    return mFilter;
}

void PasswordFilterModel::setPasswordFilter(const QString &filter)
{
    if (mFilter != filter) {
        mFilter = filter;
        mParts = filter.splitRef(QLatin1Char('/'), QString::SkipEmptyParts);
        Q_EMIT passwordFilterChanged();
        mSortingLookup.clear();
        invalidate();
    }
}

QVariant PasswordFilterModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        return data(index, PasswordsModel::FullNameRole);
    }

    return QSortFilterProxyModel::data(index, role);
}


bool PasswordFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto src_index = sourceModel()->index(source_row, 0, source_parent);
    const auto type = static_cast<PasswordsModel::EntryType>(sourceModel()->data(src_index, PasswordsModel::EntryTypeRole).toInt());
    if (type == PasswordsModel::FolderEntry) {
        return false;
    }

    if (mFilter.isEmpty()) {
        return true;
    }

    const auto path = sourceModel()->data(src_index, PasswordsModel::FullNameRole).toString();

    const auto weight = matchPathFilter(path.splitRef(QLatin1Char('/')), mParts);
    if (weight > -1) {
        mSortingLookup.insert(src_index, weight);
        return true;
    }

    return false;
}

bool PasswordFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const auto weightLeft = mSortingLookup.value(source_left, -1);
    const auto weightRight = mSortingLookup.value(source_right, -1);

    if (weightLeft == weightRight) {
        const auto nameLeft = source_left.data(PasswordsModel::FullNameRole).toString();
        const auto nameRight = source_right.data(PasswordsModel::FullNameRole).toString();
        return QString::localeAwareCompare(nameLeft, nameRight) < 0;
    }

    return weightLeft < weightRight;
}
