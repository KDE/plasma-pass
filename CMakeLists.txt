cmake_minimum_required(VERSION 2.8.12)

project(plasma-plasmapass)

set(PROJECT_VERSION "0.1")
set(PROJECT_VERSION_MAJOR 0)

set(QT_MIN_VERSION "5.9")
set(KF5_MIN_VERSION "5.42.0")

set(CMAKE_CXX_STANDARD 14)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Plasma
    I18n
    ItemModels
)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Gui
    Qml
)

# plasmoid
plasma_install_package(package org.kde.plasma.pass)

# qml extension plugin
add_subdirectory(plugin)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
install( FILES plasma-pass.categories DESTINATION ${KDE_INSTALL_CONFDIR} )
