/***************************************************************************
 *   Copyright (C) 2018  Daniel Vrátil <dvratil@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kirigami 2.0 // for Units

import org.kde.plasma.private.plasmapass 1.0

PlasmaComponents.ListItem {
    id: root

    property alias name: label.text
    property string icon

    property PasswordProvider password: null

    signal itemSelected(var index)

    // the 1.6 comes from ToolButton's default height
    height: Math.max(row.height, Math.round(units.gridUnit * 1.6)) + 2 * units.smallSpacing

    enabled: true

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
        target: root.password
        onValidChanged: {
            if (wasValid && !target.valid) {
                root.password = null;
            } else if (!wasValid && target.valid) {
                wasValid = true;
                // Password has become valid, we can close the applet
                plasmoid.expanded = false;
            }
        }
    }


    RowLayout {
        spacing: Units.largeSpacing
        id: row

        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
        }

        PlasmaCore.IconItem {
            id: entryTypeIcon
            animated: root.password != null && password.valid
            source: root.password == null ? root.icon : (password.valid ? "dialog-ok" : "process-working")
            width: Units.iconSizes.small
            height: Units.iconSizes.small
        }

        ColumnLayout {
            Layout.fillWidth: true

            PlasmaComponents.Label {
                id: label

                height: undefined // unset PlasmaComponents.Label default height

                Layout.fillWidth: true

                maximumLineCount: 1
                verticalAlignment: Text.AlignLeft
                elide: Text.ElideRight
                textFormat: Text.StyledText
            }

            PlasmaComponents.ProgressBar {
                id: passwordTimeoutBar

                Layout.fillWidth: true

                visible: root.password != null && password.valid

                minimumValue: 0
                maximumValue: root.password == null ? 0 : password.defaultTimeout
                value: root.password == null ? 0 : password.timeout
            }
        }
    }
}

