# SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
#
# SPDX-License-Identifier: LGPL-2.1-or-later

cmake_minimum_required(VERSION 3.5)

set(PROJECT_VERSION "1.2.0")

project(plasma-plasmapass VERSION ${PROJECT_VERSION})

set(PROJECT_VERSION_MAJOR 1)

set(QT_MIN_VERSION "5.11")
set(KF5_MIN_VERSION "5.57.0")

set(CMAKE_CXX_STANDARD 14)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(KDEClangFormat)
if(ECM_VERSION VERSION_GREATER_EQUAL 5.79)
    include(KDEGitCommitHooks)
endif()

find_package(OATH REQUIRED)
find_package(QGpgme REQUIRED)

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
    Concurrent
)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)
add_definitions(-DQT_NO_FOREACH)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")

# plasmoid
plasma_install_package(package org.kde.plasma.pass)

# qml extension plugin
add_subdirectory(plugin)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
if(ECM_VERSION VERSION_GREATER_EQUAL 5.59)
    install(FILES plasma-pass.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
else()
    install(FILES plasma-pass.categories DESTINATION ${KDE_INSTALL_CONFDIR})
endif()

if(ECM_VERSION VERSION_GREATER_EQUAL 5.79)
    kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

