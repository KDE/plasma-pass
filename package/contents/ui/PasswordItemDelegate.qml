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

PlasmaComponents.ListItem {
    id: menuItem

    property alias name: label.text
    property alias icon: icon.source

    signal itemSelected(string uuid)

    // the 1.6 comes from ToolButton's default height
    height: Math.max(label.height, Math.round(units.gridUnit * 1.6)) + 2 * units.smallSpacing

    enabled: true

    onClicked: {
        menuItem.itemSelected(index);
        plasmoid.expanded = false;
    }

    onContainsMouseChanged: {
        if (containsMouse) {
            listView.currentIndex = index
        } else {
            listView.currentIndex = -1
        }
    }

    RowLayout {
        height: childrenRect.height
        spacing: Units.largeSpacing
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
        }

        PlasmaCore.IconItem {
            id: icon
            width: Units.iconSizes.small
            height: Units.iconSizes.small
        }

        PlasmaComponents.Label {
            id: label

            height: undefined // unset PlasmaComponents.Label default height

            Layout.fillWidth: true

            maximumLineCount: 1
            verticalAlignment: Text.AlignLeft
            elide: Text.ElideRight
            textFormat: Text.StyledText
        }
    }
}

