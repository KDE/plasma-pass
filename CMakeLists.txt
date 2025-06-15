# SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
#
# SPDX-License-Identifier: LGPL-2.1-or-later

cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION "1.2.90")

project(plasma-plasmapass VERSION ${PROJECT_VERSION})

set(PROJECT_VERSION_MAJOR 1)

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.3.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(FeatureSummary)

find_package(OATH)
set_package_properties(OATH
    PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Provides components to build one-time password authentication systems"
    PURPOSE "Generates OTP from secret stored in password files")

find_package(QGpgmeQt6 ${GPGME_REQUIRED_VERSION} CONFIG REQUIRED)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    I18n
    ItemModels
    KIO
)

find_package(Plasma REQUIRED)
find_package(Plasma5Support REQUIRED)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Gui
    Qml
    Concurrent
)
ecm_set_disabled_deprecation_versions(QT 6.5)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# plasmoid
plasma_install_package(package org.kde.plasma.pass)

# qml extension plugin
add_subdirectory(plugin)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
install(FILES plasma-pass.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

