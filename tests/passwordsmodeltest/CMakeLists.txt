include_directories(${CMAKE_SOURCE_DIR}/plugin)

set(passwordsmodeltest_SRCS
    main.cpp
    mainwindow.cpp
)

add_executable(passwordsmodeltest ${passwordsmodeltest_SRCS})
target_link_libraries(passwordsmodeltest
    plasmapass
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    KF5::Plasma
)
