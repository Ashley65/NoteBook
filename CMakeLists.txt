cmake_minimum_required(VERSION 3.16)
project(taskHelper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.10.0/msvc2022_64")
endif ()


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Svg
        REQUIRED)



set (SOURCE_FILES
        main.cpp
        src/UI/componets/Top_Menubar/windowsAction.cpp
        src/UI/mainWindow.cpp
        src/UI/componets/Top_Menubar/infoBar.cpp
        src/UI/componets/Top_Menubar/NavigationBar.cpp
        src/UI/componets/Top_Menubar/MenuButtonBar.cpp
        src/UI/componets/Side_Bar/sideBar.cpp
        src/UI/componets/Side_Bar/Components/CoreNavigationSection.cpp
        src/UI/componets/Side_Bar/Components/PrimaryActionSection.cpp
        src/UI/componets/Side_Bar/Components/ProjectsSection.cpp
        src/UI/componets/Side_Bar/Components/FiltersSection.cpp
        src/UI/componets/Side_Bar/Components/FooterSection.cpp)

set (RESOURCE_FILES
        resources/resources.qrc)


set (HEADER_FILES
        include/UI/components/Top_Menubar/topMenubar.h
        include/UI/components/Top_Menubar/windowsAction.h
        include/UI/components/Top_Menubar/NavigationBar.h
        include/UI/components/Top_Menubar/InfoBar.h
        include/UI/mainWIndow.h
        include/UI/components/Top_Menubar/MenuButtonBar.h
        include/UI/components/SIde_Bar/sideBar.h
        include/UI/components/SIde_Bar/componets/FooterSection.h
        include/UI/components/SIde_Bar/componets/FiltersSection.h
        include/UI/components/SIde_Bar/componets/ProjectsSection.h
        include/UI/components/SIde_Bar/componets/CoreNavigationSection.h
        include/UI/components/SIde_Bar/componets/PrimaryActionSection.h
        include/UI/components/SIde_Bar/componets/BaseNavTree.h
        include/UI/helpers/IconHelper.h)

add_executable(taskHelper ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES} )


target_link_libraries(taskHelper PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Svg
)

target_include_directories(taskHelper PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)


if (WIN32)
    target_link_libraries(taskHelper PRIVATE Dwmapi)
    target_compile_definitions(taskHelper PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)
endif()

if (APPLE)
    set_target_properties(taskHelper PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.chronotasks.taskhelper"
            MACOSX_BUNDLE_BUNDLE_NAME "ChronoTasks"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
            MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
    )
endif()

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets Svg)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/iconengines/")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_INSTALL_PATH}/plugins/iconengines/qsvgicon${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/iconengines/")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats/")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_INSTALL_PATH}/plugins/imageformats/qsvg${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats/")
endif ()
