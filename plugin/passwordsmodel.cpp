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

#include "passwordsmodel.h"
#include "passwordprovider.h"

#include <QDir>
#include <QDebug>

using namespace PlasmaPass;

#define PASSWORD_STORE_DIR "PASSWORD_STORE_DIR"

class PasswordsModel::Node
{
public:
    Node() {}

    Node(const QString &name, PasswordsModel::EntryType type, Node *parent)
        : name(name), type(type), parent(parent)
    {
        if (parent) {
            parent->children.append(this);
        }
    }

    ~Node()
    {
        qDeleteAll(children);
    }

    QString path() const
    {
        if (!parent) {
            return name;
        } else {
            QString fullName = name;
            if (type == PasswordsModel::PasswordEntry) {
                fullName += QStringLiteral(".gpg");
            }
            return parent->path() + QLatin1Char('/') + fullName;
        }
    }

    QString name;
    PasswordsModel::EntryType type;
    Node *parent = nullptr;
    QVector<Node*> children;
};


PasswordsModel::PasswordsModel(QObject *parent)
    : QAbstractItemModel(parent)
    , mWatcher(this)
{
    if (qEnvironmentVariableIsSet(PASSWORD_STORE_DIR)) {
        mPassStore = QDir(QString::fromUtf8(qgetenv(PASSWORD_STORE_DIR)));
    } else {
        mPassStore = QDir(QStringLiteral("%1/.password-store").arg(QDir::homePath()));
    }

    populate();
}

PasswordsModel::~PasswordsModel()
{
    delete mRoot;
}

PasswordsModel::Node *PasswordsModel::node(const QModelIndex& index) const
{
    return static_cast<Node*>(index.internalPointer());
}

QHash<int, QByteArray> PasswordsModel::roleNames() const
{
    return { { NameRole, "name" },
             { EntryTypeRole, "type" },
             { PathRole, "path" },
             { PasswordRole, "password" } };
}

int PasswordsModel::rowCount(const QModelIndex &parent) const
{
    const auto parentNode = parent.isValid() ? node(parent) : mRoot;
    return parentNode ? parentNode->children.count() : 0;
}

int PasswordsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QModelIndex PasswordsModel::index(int row, int column, const QModelIndex &parent) const
{
    const auto parentNode = parent.isValid() ? node(parent) : mRoot;
    if (!parentNode || row < 0 || row >= parentNode->children.count() || column != 0) {
        return {};
    }

    return createIndex(row, column, parentNode->children.at(row));
}

QModelIndex PasswordsModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return {};
    }

    const auto childNode = node(child);
    if (!childNode || !childNode->parent) {
        return {};
    }
    const auto parentNode = childNode->parent;
    if (parentNode == mRoot) {
        return {};
    }
    return createIndex(parentNode->parent->children.indexOf(parentNode), 0, parentNode);
}

QVariant PasswordsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    const auto node = this->node(index);
    if (!node) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        return node->name;
    case EntryTypeRole:
        return node->type;
    case PathRole:
        return node->path();
    case PasswordRole:
        return QVariant::fromValue(new PasswordProvider(node->path()));
    }

    return {};
}

void PasswordsModel::populate()
{
    beginResetModel();
    delete mRoot;
    mRoot = new Node;
    mRoot->name = mPassStore.absolutePath();
    populateDir(mPassStore, mRoot);
    endResetModel();
}

void PasswordsModel::populateDir(const QDir& dir, Node *parent)
{
    auto entries = dir.entryInfoList({ QStringLiteral("*.gpg") }, QDir::Files, QDir::NoSort);
    for (const auto &entry : qAsConst(entries)) {
        new Node(entry.completeBaseName(), PasswordEntry, parent);
    }
    entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);
    for (const auto &entry : qAsConst(entries)) {
        auto node = new Node(entry.baseName(), FolderEntry, parent);
        populateDir(entry.absoluteFilePath(), node);
    }
}
