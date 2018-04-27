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

#ifndef PASSWORDSMODEL_H_
#define PASSWORDSMODEL_H_

#include <QAbstractItemModel>
#include <QFileSystemWatcher>
#include <QDir>

namespace PlasmaPass {

class PasswordsModel : public QAbstractItemModel
{
    Q_OBJECT

    class Node;
public:
    enum EntryType {
        FolderEntry,
        PasswordEntry
    };
    Q_ENUM(EntryType)

    enum Roles {
        NameRole = Qt::DisplayRole,
        EntryTypeRole = Qt::UserRole,
        PathRole,
        PasswordRole,
        HasPasswordRole
    };

    explicit PasswordsModel(QObject *parent = nullptr);
    ~PasswordsModel() override;

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex & parent) const override;
    int columnCount(const QModelIndex & parent) const override;

    QModelIndex index(int row, int column, const QModelIndex & parent) const override;
    QModelIndex parent(const QModelIndex & child) const override;

    QVariant data(const QModelIndex &index, int role) const override;

private:
    void populate();
    void populateDir(const QDir &dir, Node *parent);

    Node *node(const QModelIndex &index) const;

    QFileSystemWatcher mWatcher;
    QDir mPassStore;

    Node *mRoot = nullptr;
};

}
#endif
