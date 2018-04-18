include_directories(${CMAKE_SOURCE_DIR}/plugin)

set(passwordsmodeltest_SRCS
    main.cpp
    mainwindow.cpp
    ${CMAKE_SOURCE_DIR}/plugin/passwordsmodel.cpp
    ${CMAKE_SOURCE_DIR}/plugin/passwordprovider.cpp
)

add_executable(passwordsmodeltest ${passwordsmodeltest_SRCS})
target_link_libraries(passwordsmodeltest Qt5::Core Qt5::Widgets)
