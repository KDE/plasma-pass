// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents

import org.kde.plasma.private.plasmapass

import org.kde.kirigami as Kirigami

MouseArea {
    id: root

    property alias name: label.text
    property string icon
    property var entryType

    property PasswordProvider passwordProvider: null
    property OTPProvider otpProvider: null

    property alias provider: root.passwordProvider

    signal itemSelected(var index)
    signal otpClicked(var index)

    enabled: true

    implicitHeight: Math.max(column.height, otpButton.implicitHeight + 2 * Kirigami.Units.smallSpacing)

    onClicked: {
        root.itemSelected(index);
    }

    onContainsMouseChanged: {
        if (containsMouse) {
            listView.currentIndex = index
        } else {
            listView.currentIndex = -1
        }
    }

    // When password becomes invalid again, forget about it
    Connections {
        property bool wasValid : false
        target: root.provider
        function onValidChanged() {
            if (wasValid && !target.valid) {
                root.provider = null;
            } else if (!wasValid && target.valid) {
                wasValid = true;
                // Password has become valid, wait a little bit and then close the plasmoid
                hideTimer.start()
            }
        }
    }

    Timer {
        id: hideTimer
        interval: Kirigami.Units.longDuration
        onTriggered: plasmoid.expanded = false;
    }

    Column {
        id: column
        spacing: Kirigami.Units.smallSpacing
        anchors {
            left: parent.left
            right: parent.right
        }

        RowLayout {
            spacing: Kirigami.Units.largeSpacing
            id: row

            width: parent.width

            Kirigami.Icon {
                id: entryTypeIcon
                visible: root.provider == null || root.provider.valid || root.provider.hasError
                source: {
                    if (root.provider == null) {
                        return root.icon;
                    } else {
                        if (root.provider.hasError) {
                            return "dialog-error";
                        } else {
                            return "dialog-ok";
                        }
                    }
                }
                width: Kirigami.Units.iconSizes.small
                height: Kirigami.Units.iconSizes.small
            }

            PlasmaComponents.BusyIndicator {
                id: busyIndicator
                visible: root.provider != null && !root.provider.valid && !root.provider.hasError

                // Hack around BI wanting to be too large by default
                Layout.maximumWidth: entryTypeIcon.width
                Layout.maximumHeight: entryTypeIcon.height
            }

            PlasmaComponents.Label {
                id: label

                height: undefined // unset PlasmaComponents.Label default height

                Layout.fillWidth: true

                maximumLineCount: 1
                verticalAlignment: Text.AlignLeft
                elide: Text.ElideRight
                textFormat: Text.PlainText
            }

            PlasmaComponents.ToolButton {
                id: otpButton
                icon.name: 'clock'
                visible: entryType == PasswordsModel.PasswordEntry

                PlasmaComponents.ToolTip {
                    text: i18n("One-time password (OTP)")
                }

                onClicked: {
                    root.otpClicked(index);
                }
            }
        }

        ProviderDelegate {
            id: passwordDelegate
            provider: root.passwordProvider
            icon: 'lock'
            visible: root.passwordProvider !== null
            width: parent.width
        }

        ProviderDelegate {
            id: otpDelegate
            provider: root.otpProvider
            icon: 'clock'
            visible: root.otpProvider !== null
            width: parent.width
        }
    }
}

