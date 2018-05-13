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
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kirigami 2.0 // for Units

import org.kde.plasma.private.plasmapass 1.0

Item {
    Plasmoid.fullRepresentation: ColumnLayout {
        anchors.fill: parent

        Connections {
            target: plasmoid
            onExpandedChanged: {
                if (!plasmoid.expanded) {
                    viewStack.reset();
                } else {
                    filterField.focus = true
                }
            }
        }

        PasswordSortProxyModel {
            id: passwordsModel

            dynamicSortFilter: true
            isSortLocaleAware: true
            sortCaseSensitivity: Qt.CaseInsensitive

            sourceModel: PasswordsModel {}
        }

        PasswordFilterModel {
            id: filterModel

            filter: filterField.text

            sourceModel: passwordsModel
        }

        Component {
            id: passwordsPage

            PasswordsPage {
                stack: viewStack
                model: filterModel.filter == "" ? passwordsModel : filterModel
                onItemSelected: {
                    stack.pushPage(index, name);
                }
            }
        }

        RowLayout {
            PlasmaComponents.ToolButton {
                iconSource: "draw-arrow-back"
                onClicked: viewStack.popPage()
                enabled: viewStack.depth > 1
            }

            PlasmaComponents.Label {
                id: currentPath

                Layout.fillWidth: true

                property var _path: []

                function pushName(name) {
                    _path.push(name);
                    text = _path.join("/");
                }
                function popName() {
                    _path.pop();
                    text = _path.join("/");
                }
            }
        }

        RowLayout {
            PlasmaComponents.TextField {
                id: filterField

                placeholderText: i18n("Filter...")
                clearButtonShown: true

                Layout.fillWidth: true
            }
        }

        PlasmaComponents.PageStack {
            id: viewStack

            Layout.fillHeight: true
            Layout.fillWidth: true

            function pushPage(index, name) {
                push(passwordsPage.createObject(viewStack, { "rootIndex": index, "stack": viewStack }));
                currentPath.pushName(name);
            }

            function popPage() {
                pop();
                currentPath.popName();
            }

            function clear() {
                while (depth > 1) {
                    popPage();
                }
            }

            Component.onCompleted: {
                initialPage = passwordsPage.createObject(viewStack);
            }
        }
    }
}
