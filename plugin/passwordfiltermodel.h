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
#include <QVector>
#include <QTimer>
#include <QFuture>

class QStringRef;
class KDescendantsProxyModel;

namespace PlasmaPass {

class PasswordFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString passwordFilter READ passwordFilter WRITE setPasswordFilter NOTIFY passwordFilterChanged)
public:
    explicit PasswordFilterModel(QObject *parent = nullptr);

    void setSourceModel(QAbstractItemModel *sourceModel) override;

    QString passwordFilter() const;
    void setPasswordFilter(const QString &filter);

    QVariant data(const QModelIndex &index, int role) const override;

Q_SIGNALS:
    void passwordFilterChanged();

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    struct PathFilter {
        using result_type = std::pair<QModelIndex, int>;

        explicit PathFilter() = default;
        PathFilter(const QString &filter);

        PathFilter(const PathFilter &);
        PathFilter(PathFilter &&) noexcept;
        PathFilter &operator=(const PathFilter &);
        PathFilter &operator=(PathFilter &&) noexcept;

        result_type operator()(const QModelIndex &index) const;

        QString filter;
    private:
        void updateParts();
        QVector<QStringRef> mParts;
    };

    void delayedUpdateFilter();

    KDescendantsProxyModel *mFlatModel = nullptr;
    PathFilter mFilter;
    mutable QHash<QModelIndex, int> mSortingLookup;
    QTimer mUpdateTimer;
    QFuture<QHash<QModelIndex, int>> mFuture;
};

}

#endif
