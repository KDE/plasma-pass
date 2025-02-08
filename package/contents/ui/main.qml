// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2 // For StackView
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.core as PlasmaCore

import org.kde.plasma.private.plasmapass

import org.kde.kirigami as Kirigami

PlasmoidItem {
    id: root

    fullRepresentation: PlasmaExtras.Representation {
        collapseMarginsHint: true
        Layout.minimumWidth: Kirigami.Units.gridUnit * 5
        Layout.minimumHeight: Kirigami.Units.gridUnit * 5

        property bool expanded: false

        Component.onCompleted: {
            // FIXME: I'm probably doing something wrong, but I'm unable to access
            // "plasmoid" from elsewhere
            expanded = Qt.binding(function() { return root.expanded; });
        }

        Keys.onPressed: event=> {
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
                viewStack.pop(null);
                currentPath.clearName();
            }
        }
        header: PlasmaExtras.PlasmoidHeading {
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
                        onFolderSelected: (index, name)=> {
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
                    visible: viewStack.depth > 1 && !viewStack.filterMode
                    PlasmaComponents.ToolButton {
                        icon.name: LayoutMirroring.enabled ? "go-previous-symbolic-rtl" : "go-previous-symbolic"
                        onClicked: viewStack.popPage()
                        enabled: viewStack.depth > 1
                    }

                    PlasmaComponents.Label {
                        id: currentPath

                        Layout.fillWidth: true
                        HoverHandler {
                            id: hoverHandler
                        }
                        PlasmaComponents.ToolTip {
                            text: currentPath.text
                            visible: hoverHandler.hovered
                        }

                        property var _path: []

                        function pushName(name) {
                            _path.push(name);
                            text = _path.join("/");
                        }
                        function popName() {
                            _path.pop();
                            text = _path.join("/");
                        }
                        function clearName() {
                            _path = []
                        }
                    }
                }

                PlasmaComponents.TextField {
                    id: filterField
                    focus: true
                    activeFocusOnTab: true

                    placeholderText: i18n("Filter...")
                    clearButtonShown: true

                    Layout.fillWidth: true

                    Keys.priority: Keys.BeforeItem
                    Keys.onPressed: event=> {
                        if (event.key == Qt.Key_Down) {
                            viewStack.focus = true;
                            event.accepted = true;
                        } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                            viewStack.currentItem.activateCurrentItem();
                            event.accepted = true;
                        }
                    }
                }
            }
        }

        QQC2.StackView {
            id: viewStack
            anchors.fill: parent

            readonly property bool filterMode: filterField.text !== ""

            onCurrentItemChanged: {
                if (currentItem) {
                    currentItem.focus = true;
                    currentItem.forceActiveFocus();
                }
            }

            onFilterModeChanged: {
                pop(null);
                if (filterMode) {
                    push(filterPage.createObject(viewStack, { rootIndex: null, stack: viewStack }));
                }
                // Keep focus on the filter field
                filterField.focus = true;
                filterField.forceActiveFocus();
            }

            function pushPage(index, name) {
                const newPage = passwordsPage.createObject(viewStack, { rootIndex: index, stack: viewStack });
                push(newPage);
                currentPath.pushName(name);
            }

            function popPage() {
                pop();
                currentPath.popName();
            }
            initialItem: passwordsPage
        }
    }
}
