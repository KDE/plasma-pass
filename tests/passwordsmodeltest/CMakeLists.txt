find_package(Qt5Widgets CONFIG REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/plugin)

set(passwordsmodeltest_SRCS
    main.cpp
    mainwindow.cpp
)

add_executable(passwordsmodeltest ${passwordsmodeltest_SRCS})
target_link_libraries(passwordsmodeltest
    plasmapass
    Qt::Core
    Qt::DBus
    Qt::Widgets
    KF5::Plasma
)
