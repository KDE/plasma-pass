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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QPointer>

class QLabel;
class QPushButton;
class QProgressBar;
class QStackedWidget;

namespace PlasmaPass
{
class PasswordProvider;
class PasswordFilterModel;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private Q_SLOTS:
    void onPasswordClicked(const QModelIndex &idx);
    void onSearchChanged(const QString &text);

private:
    void setProvider(PlasmaPass::PasswordProvider *provider);

    QLabel *mTitle = nullptr;
    QLabel *mType = nullptr;
    QLabel *mPath = nullptr;
    QLabel *mPassword = nullptr;
    QLabel *mError = nullptr;
    QPushButton *mPassBtn = nullptr;
    QProgressBar *mPassProgress = nullptr;
    QModelIndex mCurrent;
    QPointer<PlasmaPass::PasswordProvider> mProvider;
    QStackedWidget *mStack = nullptr;
    PlasmaPass::PasswordFilterModel *mFilterModel = nullptr;
};

#endif
