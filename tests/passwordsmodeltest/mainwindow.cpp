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

#include "mainwindow.h"
#include "passwordsmodel.h"
#include "passwordprovider.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QTreeView>
#include <QLabel>
#include <QPushButton>

using namespace PlasmaPass;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *w = new QWidget;
    setCentralWidget(w);

    auto h = new QHBoxLayout(w);

    auto treeView = new QTreeView;
    treeView->setModel(new PasswordsModel(this));
    connect(treeView, &QTreeView::clicked, this, &MainWindow::onPasswordClicked);
    h->addWidget(treeView);

    w = new QWidget;
    h->addWidget(w);

    auto v = new QVBoxLayout(w);

    v->addWidget(mTitle = new QLabel);
    auto font = mTitle->font();
    font.setBold(true);
    mTitle->setFont(font);

    v->addWidget(mPath = new QLabel);

    auto g = new QFormLayout;
    v->addLayout(g);
    g->addRow(QStringLiteral("Type:"), mType = new QLabel());
    g->addRow(QStringLiteral("Password:"), mPassword = new QLabel());

    v->addWidget(mPassBtn = new QPushButton(QStringLiteral("Display Password")));
    connect(mPassBtn, &QPushButton::clicked,
            this, [this]() {
                mPassBtn->setVisible(false);
                auto provider = mCurrent.data(PasswordsModel::PasswordRole).value<PasswordProvider*>();
                connect(provider, &PasswordProvider::passwordChanged,
                        this, [this, provider]() {
                            mPassword->setVisible(true);
                            mPassword->setText(provider->password());
                        });
            });

    v->addStretch(2.0);

    onPasswordClicked({});
}

MainWindow::~MainWindow()
{
}

void MainWindow::onPasswordClicked(const QModelIndex &idx)
{
    mCurrent = idx;
    mTitle->setText(idx.data(PasswordsModel::NameRole).toString());
    mPath->setText(idx.data(PasswordsModel::PathRole).toString());
    const auto type = idx.isValid() ? static_cast<PasswordsModel::EntryType>(idx.data(PasswordsModel::EntryTypeRole).toInt()) : PasswordsModel::FolderEntry;
    mType->setText(type == PasswordsModel::PasswordEntry ? QStringLiteral("Password") : QStringLiteral("Folder"));
    mPassword->setVisible(false);
    mPassword->setText({});
    mPassBtn->setEnabled(type == PasswordsModel::PasswordEntry);
    mPassBtn->setVisible(true);
}
