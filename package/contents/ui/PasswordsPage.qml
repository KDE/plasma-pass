// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick 2.1
import QtQml.Models 2.1

import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.private.plasmapass 1.0

PlasmaExtras.ScrollArea {
    id: scroll

    signal folderSelected(var index, var name)

    function activateCurrentItem() {
        if (listView.currentItem) {
            listView.currentItem.activate();
        }
    }

    property Item stack
    property var rootIndex: null
    property alias model: delegateModel.model

    focus: true

    ListView {
        id: listView

        onActiveFocusChanged: {
            if (activeFocus && listView.currentIndex === -1) {
                listView.currentIndex = 0;
            }
        }

        Keys.onPressed: {
            if (event.key === Qt.Key_Down) {
                if (listView.currentIndex < listView.count - 1) {
                    listView.currentIndex++;
                }
                event.accepted = true;
            } else if (event.key === Qt.Key_Up) {
                if (listView.currentIndex > 0) {
                    listView.currentIndex--;
                }
                event.accepted = true;
            } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                if (listView.currentItem) {
                    listView.currentItem.activate();
                }
                event.accepted = true;
            }
        }

        focus: true
        activeFocusOnTab: true
        highlightFollowsCurrentItem: true

        model: DelegateModel {
            id: delegateModel

            rootIndex: scroll.rootIndex

            delegate: PasswordItemDelegate {
                id: delegate

                name: model.name
                icon: model.type === PasswordsModel.FolderEntry ? "inode-directory" : "lock"
                entryType: model.type

                passwordProvider: model.hasPassword ? model.password : null
                otpProvider: model.hasOTP ? model.otp : null

                onItemSelected: activate();
                onOtpClicked: function() {
                    delegate.otpProvider = model.otp
                }

                function activate() {
                    if (model.type === PasswordsModel.FolderEntry) {
                        scroll.folderSelected(delegateModel.modelIndex(index), model.name)
                    } else {
                        delegate.passwordProvider = model.password
                    }
                }
            }
        }

        boundsBehavior: Flickable.StopAtBounds
        interactive: contentHeight > height
        highlight: PlasmaComponents.Highlight { }
        highlightMoveDuration: 0
        highlightResizeDuration: 0
        currentIndex: -1
    }
}
