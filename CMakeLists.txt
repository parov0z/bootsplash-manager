cmake_minimum_required(VERSION 3.5)

project(bootsplash-manager LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(KF5Auth REQUIRED)
find_package(fmt REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(PAMAC REQUIRED pamac)

add_executable(${PROJECT_NAME}
    src/bootsplash-manager.cpp
    src/root-actions.h
    src/root-actions.cpp
)

set(TS_FILES
    data/translations/${PROJECT_NAME}_ru_RU.ts
)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

add_executable(${PROJECT_NAME}-gui
    ${QM_FILES}
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

add_executable(${PROJECT_NAME}-gui-helper
    src/helper/helper.cpp
    src/helper/helper.h
    src/root-actions.h
    src/root-actions.cpp
)

target_include_directories(${PROJECT_NAME}-gui-helper PUBLIC include ${GLIB2_INCLUDE_DIRS} ${PAMAC_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}-gui        PUBLIC include ${GLIB2_INCLUDE_DIRS} ${PAMAC_INCLUDE_DIRS})

target_compile_definitions(${PROJECT_NAME}-gui-helper PUBLIC -DQT_NO_KEYWORDS)
target_compile_definitions(${PROJECT_NAME}-gui        PUBLIC -DQT_NO_KEYWORDS)

target_link_libraries(${PROJECT_NAME}            Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(${PROJECT_NAME}-gui        PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${SDL2_LIBRARIES} ${GLIB2_LIBRARIES} ${PAMAC_LIBRARIES} KF5::Auth)
target_link_libraries(${PROJECT_NAME}-gui-helper Qt${QT_VERSION_MAJOR}::Core ${GLIB2_LIBRARIES} ${PAMAC_LIBRARIES} KF5::Auth )

target_include_directories(${PROJECT_NAME}-gui PRIVATE ${SDL2_INCLUDE_DIRS} .)

find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(GNUInstallDirs)
include(ECMInstallIcons)

install(TARGETS ${PROJECT_NAME}             DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS ${PROJECT_NAME}-gui         DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS ${PROJECT_NAME}-gui-helper  DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/translations)
install(FILES data/bootsplash-manager.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
ecm_install_icons(ICONS images/sc-apps-bootsplash-manager-gui.svg DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)

kauth_install_helper_files(${PROJECT_NAME}-gui-helper dev.android7890.bootsplashmanager root)
kauth_install_actions(dev.android7890.bootsplashmanager src/helper/bootsplashmanager.actions)
