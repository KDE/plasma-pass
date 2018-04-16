set(plasmapassplugin_SRCS
    plasmapassplugin.cpp
)

add_library(plasmapassplugin SHARED ${plasmapassplugin_SRCS})

target_link_libraries(plasmapassplugin
    Qt5::Gui
    Qt5::Qml
)
install(TARGETS plasmapassplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/plasmapass)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/plasmapass)
