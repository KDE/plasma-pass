// Copyright (C) 2018  Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kirigami 2.0 // for Units

import org.kde.plasma.private.plasmapass 1.0

Item {
    id: root

    Plasmoid.fullRepresentation: FocusScope {

        Layout.minimumWidth: units.gridUnit * 5
        Layout.minimumHeight: units.gridUnit * 5

        property bool expanded: false

        Component.onCompleted: {
            // FIXME: I'm probably doing something wrong, but I'm unable to access
            // "plasmoid" from elsewhere
            expanded = Qt.binding(function() { return plasmoid.expanded; });
        }

        Keys.onPressed: {
            if (!viewStack.filterMode && event.key === Qt.Key_Backspace) {
                viewStack.popPage();
                event.accepted = true;
            }
        }

        onExpandedChanged: {
            if (expanded) {
                filterField.focus = true;
                filterField.forceActiveFocus();
            } else {
                filterField.text = "";
                viewStack.clear();
            }
        }

        ColumnLayout {
            anchors.fill: parent

            PasswordSortProxyModel {
                id: passwordsModel

                dynamicSortFilter: true
                isSortLocaleAware: true
                sortCaseSensitivity: Qt.CaseInsensitive

                sourceModel: PasswordsModel {}
            }

            PasswordFilterModel {
                id: filterModel

                passwordFilter: filterField.text

                sourceModel: passwordsModel
            }

            Component {
                id: passwordsPage

                PasswordsPage {
                    stack: viewStack
                    model: passwordsModel
                    onFolderSelected: {
                        stack.pushPage(index, name);
                    }
                }
            }

            Component {
                id: filterPage

                PasswordsPage {
                    stack: viewStack
                    model: filterModel
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
                    focus: true
                    activeFocusOnTab: true

                    placeholderText: i18n("Filter...")
                    clearButtonShown: true

                    Layout.fillWidth: true

                    Keys.priority: Keys.BeforeItem
                    Keys.onPressed: {
                        if (event.key == Qt.Key_Down) {
                            viewStack.currentPage.focus = true;
                            event.accepted = true;
                        } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                            viewStack.currentPage.activateCurrentItem();
                            event.accepted = true;
                        }
                    }
                }
            }

            PlasmaComponents.PageStack {
                id: viewStack

                Layout.fillHeight: true
                Layout.fillWidth: true

                readonly property bool filterMode: filterField.text !== ""

                onCurrentPageChanged: {
                    currentPage.focus = true;
                    currentPage.forceActiveFocus();
                }

                onFilterModeChanged: {
                    clear();
                    if (filterMode) {
                        push(filterPage.createObject(viewStack, { "rootIndex": null, "stack": viewStack }));
                    }
                    // Keep focus on the filter field
                    filterField.focus = true;
                }

                function pushPage(index, name) {
                    var newPage = passwordsPage.createObject(viewStack, { "rootIndex": index, "stack": viewStack });
                    push(newPage);
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
}
