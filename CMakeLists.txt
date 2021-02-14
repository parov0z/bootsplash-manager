cmake_minimum_required(VERSION 3.5)

project(bootsplash-manager LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

add_executable(bootsplash-manager
    src/bootsplash-manager.cpp
)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(bootsplash-manager-gui
        src/bootsplash-manager-gui.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        images/icons.qrc
    )
else()
    add_executable(bootsplash-manager-gui
        src/bootsplash-manager-gui.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        images/icons.qrc
    )
endif()

target_link_libraries(bootsplash-manager-gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(bootsplash-manager Qt${QT_VERSION_MAJOR}::Core)
