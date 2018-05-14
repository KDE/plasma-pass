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

import QtQuick 2.1
import QtQml.Models 2.1

import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.private.plasmapass 1.0

PlasmaExtras.ScrollArea {
    id: scroll

    signal folderSelected(var index, var name)

    property Item stack
    property var rootIndex: null
    property alias model: delegateModel.model

    focus: true

    ListView {
        id: listView

        onActiveFocusChanged: {
            if (activeFocus && listView.currentIndex == -1) {
                listView.currentIndex = 0;
            }
        }

        Keys.onPressed: {
            if (event.key == Qt.Key_Down) {
                if (listView.currentIndex < listView.count - 1) {
                    listView.currentIndex++;
                }
                event.accepted = true;
            } else if (event.key == Qt.Key_Up) {
                if (listView.currentIndex > 0) {
                    listView.currentIndex--;
                }
                event.accepted = true;
            } else if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
                if (listView.currentItem != null) {
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

                width: parent.parent.width

                name: model.name
                icon: model.type == PasswordsModel.FolderEntry ? "inode-directory" : "lock"
                entryType: model.type

                onItemSelected: activate();

                function activate() {
                    if (model.type == PasswordsModel.FolderEntry) {
                        scroll.folderSelected(delegateModel.modelIndex(index), model.name)
                    } else {
                        delegate.password = model.password
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
