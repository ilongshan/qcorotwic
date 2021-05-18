set(qcorotwic_SRCS
    qml.qrc
    api.cpp
    authmanager.cpp
    main.cpp)


add_executable(qcorotwic ${qcorotwic_SRCS})
target_include_directories(qcorotwic PRIVATE $<BUILD_INTERFACE:${QCoro_INCLUDE_DIR}>)
target_link_libraries(qcorotwic
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VRESION_MAJOR}::Network
)
