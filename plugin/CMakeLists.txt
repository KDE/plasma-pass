# SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
#
# SPDX-License-Identifier: LGPL-2.1-or-later

###### STATIC LIBRARY ######

set(plasmapasslib_SRCS
    abbreviations.cpp
    klipperutils.cpp
    otpprovider.cpp
    providerbase.cpp
    passwordfiltermodel.cpp
    passwordsmodel.cpp
    passwordsortproxymodel.cpp
    passwordprovider.cpp

    abbreviations.h
    klipperutils.h
    otpprovider.h
    providerbase.h
    passwordfiltermodel.h
    passwordsmodel.h
    passwordsortproxymodel.h
    passwordprovider.h
)

qt_add_dbus_interfaces(plasmapasslib_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.kde.klipper.klipper.xml
)

ecm_qt_declare_logging_category(plasmapasslib_SRCS
    HEADER plasmapass_debug.h
    IDENTIFIER PlasmaPass::PLASMAPASS_LOG
    CATEGORY_NAME org.kde.plasma.pass
)

add_library(plasmapass STATIC ${plasmapasslib_SRCS})
set_property(TARGET plasmapass PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(plasmapass
    Qt::Core
    Qt::DBus
    Qt::Qml
    Qt::Concurrent
    KF6::I18n
    KF6::ItemModels
    OATH::OATH
)

target_link_libraries(plasmapass QGpgmeQt6 Plasma::Plasma5Support Qt::Gui KF6::ConfigCore KF6::CoreAddons)


########### PLUGN ###########

set(plasmapassplugin_SRCS
    plasmapassplugin.cpp
    plasmapassplugin.h
)

add_library(plasmapassplugin SHARED ${plasmapassplugin_SRCS})
target_link_libraries(plasmapassplugin
    plasmapass
)


install(TARGETS plasmapassplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/plasmapass)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/plasmapass)
