# SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
#
# SPDX-License-Identifier: LGPL-2.1-or-later

###### STATIC LIBRARY ######

set(plasmapasslib_SRCS
    abbreviations.cpp
    otpprovider.cpp
    providerbase.cpp
    passwordfiltermodel.cpp
    passwordsmodel.cpp
    passwordsortproxymodel.cpp
    passwordprovider.cpp
)

qt5_add_dbus_interfaces(plasmapasslib_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.kde.klipper.klipper.xml
)

ecm_qt_declare_logging_category(plasmapasslib_SRCS
    HEADER plasmapass_debug.h
    IDENTIFIER PlasmaPass::PLASMAPASS_LOG
    CATEGORY_NAME org.kde.plasma.pass
)

add_library(plasmapass STATIC ${plasmapasslib_SRCS})

target_link_libraries(plasmapass
    Qt::Core
    Qt::DBus
    Qt::Qml
    Qt::Concurrent
    KF5::Plasma
    KF5::I18n
    KF5::ItemModels
    OATH::OATH
    QGpgme
)

########### PLUGN ###########

set(plasmapassplugin_SRCS
    plasmapassplugin.cpp
)

add_library(plasmapassplugin SHARED ${plasmapassplugin_SRCS})
target_link_libraries(plasmapassplugin
    plasmapass
)


install(TARGETS plasmapassplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/plasmapass)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/plasmapass)
