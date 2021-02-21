// Copyright (C) 2021  Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kirigami 2.0 // for Units

import org.kde.plasma.private.plasmapass 1.0

RowLayout {
    id: root

    property ProviderBase provider: null
    property alias icon: providerIcon.source

    PlasmaCore.IconItem {
        id: providerIcon
        width: Units.iconSizes.small
        height: Units.iconSizes.small
    }

    ColumnLayout {
        PlasmaComponents.ProgressBar {
            id: timeoutBar

            Layout.fillWidth: true

            visible: root.provider != null && root.provider.valid

            minimumValue: 0
            maximumValue: root.provider == null ? 0 : root.provider.defaultTimeout
            value: root.provider == null ? 0 : root.provider.timeout
        }

        PlasmaComponents.Label {
            id: errorLabel

            height: undefined

            Layout.fillWidth: true

            visible: root.provider != null && root.provider.hasError
            text: root.provider != null ? root.provider.error : ""
            wrapMode: Text.WordWrap
        }
    }
}
