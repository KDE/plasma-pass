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

#ifndef PASSWORDFILTERMODEL_H_
#define PASSWORDFILTERMODEL_H_

#include <QSortFilterProxyModel>

class KDescendantsProxyModel;

namespace PlasmaPass {

class PasswordFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
public:
    explicit PasswordFilterModel(QObject *parent = nullptr);

    void setSourceModel(QAbstractItemModel *sourceModel) override;

    QString filter() const;
    void setFilter(const QString &filter);

    QVariant data(const QModelIndex &index, int role) const override;

Q_SIGNALS:
    void filterChanged();

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    KDescendantsProxyModel *mFlatModel = nullptr;
    QString mFilter;
    QStringList mParts;
    mutable QHash<QModelIndex, int> mSortingLookup;
};

}

#endif
