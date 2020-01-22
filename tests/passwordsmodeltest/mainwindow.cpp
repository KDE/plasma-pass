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

#include "mainwindow.h"
#include "passwordsmodel.h"
#include "passwordprovider.h"
#include "passwordfiltermodel.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QTreeView>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QListView>
#include <QLineEdit>

using namespace PlasmaPass;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(900, 350);

    QWidget *w = new QWidget;
    setCentralWidget(w);

    auto h = new QHBoxLayout(w);

    auto splitter = new QSplitter;
    h->addWidget(splitter);

    w = new QWidget;
    splitter->addWidget(w);

    auto v = new QVBoxLayout(w);

    auto input = new QLineEdit;
    input->setClearButtonEnabled(true);
    input->setPlaceholderText(QStringLiteral("Search ..."));
    connect(input, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);
    v->addWidget(input);

    mStack = new QStackedWidget;
    v->addWidget(mStack);

    auto treeView = new QTreeView;
    treeView->setHeaderHidden(true);
    treeView->setModel(new PasswordsModel(this));
    connect(treeView, &QTreeView::clicked, this, &MainWindow::onPasswordClicked);
    mStack->addWidget(treeView);

    auto listView = new QListView;
    mFilterModel = new PasswordFilterModel(listView);
    mFilterModel->setSourceModel(treeView->model());
    listView->setModel(mFilterModel);
    connect(listView, &QListView::clicked, this, &MainWindow::onPasswordClicked);
    mStack->addWidget(listView);

    mStack->setCurrentIndex(0);

    w = new QWidget;
    splitter->addWidget(w);

    v = new QVBoxLayout(w);

    v->addWidget(mTitle = new QLabel);
    auto font = mTitle->font();
    font.setBold(true);
    mTitle->setFont(font);

    auto g = new QFormLayout;
    v->addLayout(g);
    g->addRow(QStringLiteral("Path:"), mPath = new QLabel());
    g->addRow(QStringLiteral("Type:"), mType = new QLabel());
    g->addRow(QStringLiteral("Password:"), mPassword = new QLabel());
    g->addRow(QStringLiteral("Expiration:"), mPassProgress = new QProgressBar());
    g->addRow(QStringLiteral("Error:"), mError = new QLabel());
    mPassProgress->setTextVisible(false);

    v->addWidget(mPassBtn = new QPushButton(QStringLiteral("Display Password")));
    connect(mPassBtn, &QPushButton::clicked,
            this, [this]() {
                setProvider(mCurrent.data(PasswordsModel::PasswordRole).value<PasswordProvider*>());
            });

    v->addStretch(2.0);

    onPasswordClicked({});
}

MainWindow::~MainWindow()
{
}

void MainWindow::setProvider(PasswordProvider *provider)
{
    mProvider = provider;
    if (provider->isValid()) {
        mPassBtn->setVisible(false);
        mPassword->setVisible(true);
        mPassword->setText(provider->password());
    }
    if (provider->hasError()) {
        mError->setVisible(true);
        mError->setText(provider->error());
    }

    connect(provider, &PasswordProvider::passwordChanged,
            this, [this, provider]() {
                const auto pass = provider->password();
                if (!pass.isEmpty()) {
                    mPassword->setVisible(true);
                    mPassword->setText(provider->password());
                } else {
                    onPasswordClicked(mCurrent);
                }
            });
    connect(provider, &PasswordProvider::timeoutChanged,
            this, [this, provider]() {
                mPassProgress->setVisible(true);
                mPassProgress->setMaximum(provider->defaultTimeout());
                mPassProgress->setValue(provider->timeout());
            });
    connect(provider, &PasswordProvider::errorChanged,
            this, [this, provider]() {
                mError->setVisible(true);
                mError->setText(provider->error());
            });
}


void MainWindow::onPasswordClicked(const QModelIndex &idx)
{
    if (mProvider) {
        mProvider->disconnect(this);
    }
    mCurrent = idx;
    mTitle->setText(idx.data(PasswordsModel::NameRole).toString());
    mPath->setText(idx.data(PasswordsModel::PathRole).toString());
    const auto type = idx.isValid() ? static_cast<PasswordsModel::EntryType>(idx.data(PasswordsModel::EntryTypeRole).toInt()) : PasswordsModel::FolderEntry;
    mType->setText(type == PasswordsModel::PasswordEntry ? QStringLiteral("Password") : QStringLiteral("Folder"));
    mPassword->setVisible(false);
    mPassword->clear();
    mPassBtn->setEnabled(type == PasswordsModel::PasswordEntry);
    mPassBtn->setVisible(true);
    mPassProgress->setVisible(false);
    mError->clear();
    mError->setVisible(false);

    const auto hasProvider = mCurrent.data(PasswordsModel::HasPasswordRole).toBool();
    if (hasProvider) {
        setProvider(mCurrent.data(PasswordsModel::PasswordRole).value<PasswordProvider*>());
    }
}

void MainWindow::onSearchChanged(const QString &text)
{
    mStack->setCurrentIndex(text.isEmpty() ? 0 : 1);
    mFilterModel->setPasswordFilter(text);
}
