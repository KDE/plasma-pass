// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents3

import org.kde.plasma.private.plasmapass

import org.kde.kirigami as Kirigami

RowLayout {
    id: root

    property ProviderBase provider: null
    property alias icon: providerIcon.source

    Kirigami.Icon {
        id: providerIcon
        width: Kirigami.Units.iconSizes.small
        height: Kirigami.Units.iconSizes.small
    }

    ColumnLayout {
        PlasmaComponents3.ProgressBar {
            id: timeoutBar

            Layout.fillWidth: true

            visible: root.provider != null && root.provider.valid

            from: 0
            to: root.provider == null ? 0 : root.provider.defaultTimeout
            value: root.provider == null ? 0 : root.provider.timeout
        }

        PlasmaComponents3.Label {
            id: errorLabel

            height: undefined

            Layout.fillWidth: true

            visible: root.provider != null && root.provider.hasError
            text: root.provider != null ? root.provider.error : ""
            wrapMode: Text.WordWrap
        }
    }
}
