cmake_minimum_required(VERSION 3.5)

project(bootsplash-manager LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

add_executable(bootsplash-manager
    src/bootsplash-manager.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(bootsplash-manager-gui
        src/gui/bootsplash-manager-gui.cpp
        src/gui/mainwindow.cpp
        src/gui/mainwindow.h
        src/gui/mainwindow.ui
        src/gui/installdialog.cpp
        src/gui/installdialog.h
        src/gui/installdialog.ui
        images/icons.qrc
        src/gui/bootsplash_file.h
        src/gui/bootsplash-viewer.c
    )
else()
    add_executable(bootsplash-manager-gui
        src/gui/bootsplash-manager-gui.cpp
        src/gui/mainwindow.cpp
        src/gui/mainwindow.h
        src/gui/mainwindow.ui
        src/gui/installdialog.cpp
        src/gui/installdialog.h
        src/gui/installdialog.ui
        images/icons.qrc
        src/gui/bootsplash_file.h
        src/gui/bootsplash-viewer.c
    )
endif()

target_link_libraries(bootsplash-manager-gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${SDL2_LIBRARIES})
target_link_libraries(bootsplash-manager Qt${QT_VERSION_MAJOR}::Core)

target_include_directories(bootsplash-manager-gui PRIVATE ${SDL2_INCLUDE_DIRS} .)
