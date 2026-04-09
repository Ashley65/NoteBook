cmake_minimum_required(VERSION 3.16)
project(taskHelper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.11.0/msvc2022_64")
endif ()
if (APPLE)
    if (DEFINED ENV{QT_PATH})
        set(CMAKE_PREFIX_PATH "$ENV{QT_PATH}")
    else()
        set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.11.0/macos")
    endif()
endif ()


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Svg
        Quick
        QuickWidgets
        QuickLayouts
        QuickControls2
        Core5Compat
        Network
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
        src/UI/componets/Side_Bar/Components/WorkspaceContextSection.cpp
        src/UI/componets/Side_Bar/Components/ProjectsSection.cpp
        src/UI/componets/Side_Bar/Components/FiltersSection.cpp
        src/UI/componets/Side_Bar/Components/FooterSection.cpp
        src/UI/componets/Content_Windows/MainContentView.cpp
        src/helpers/AppStateController.cpp
        src/Data/workspace/WorkspaceRepository.cpp
        src/UI/componets/Content_Windows/WorkspaceView.cpp
        src/Data/workspace/WorkspaceSwitchDialog.cpp
        src/Data/workspace/WorkspaceCreateDialog.cpp
        src/Data/workspace/WorkspaceDeleteDialog.cpp
        src/Data/workspace/WorkspaceSettingsWindow.cpp
        src/UI/componets/Content_Windows/Page/NetworkTestPage.cpp)

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
        include/UI/components/SIde_Bar/componets/WorkspaceContextSection.h
        include/UI/components/SIde_Bar/componets/BaseNavTree.h
        include/helpers/IconHelper.h
        include/helpers/AppStateController.h
        include/UI/components/Content_Windows/WorkspaceView.h
        include/helpers/Workspace.h
        include/Data/workspace/WorkspaceRepository.h
        include/UI/components/Content_Windows/MainContentView.h
        include/helpers/WorkspaceCommand.h
        include/Data/workspace/WorkspaceSwitchDialog.h
        include/Data/workspace/WorkspaceCreateDialog.h
        include/Data/workspace/WorkspaceDeleteDialog.h
        include/Data/workspace/WorkspaceSettingsWindow.h
        include/Data/workspace/Structure/Note.h
        include/Data/workspace/Structure/FileAttachment.h
        include/UI/components/Content_Windows/IWorkspaceView.h
        include/UI/components/Content_Windows/WorkspaceViewFactory.h
        include/Data/helper/validater.h
        include/UI/components/Content_Windows/page/NetworkTestPage.h
        include/Data/networkInterface.h
        include/Data/Networking/INetworkAuthService.h
        include/Data/Networking/ICloudSyncService.h
        include/Data/Networking/ILanSyncService.h
        include/Data/Networking/IAttachmentTransferService.h
        include/UI/components/Content_Windows/page/loginPage.h)

add_executable(taskHelper ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES} )


target_link_libraries(taskHelper PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Svg
        Qt6::Quick
        Qt6::QuickWidgets
        Qt6::QuickLayouts
        Qt6::QuickControls2
        Qt6::Core5Compat
        Qt6::Network

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

    # Updated: Added QuickControls2Fusion to the list
    foreach (QT_LIB Core Gui Widgets Svg Quick QuickWidgets Qml QmlModels QmlMeta QmlWorkerScript OpenGL Network QuickLayouts QuickControls2 QuickTemplates2 ShaderTools QuickShapes QuickControls2Impl QuickControls2Basic QuickControls2Fusion)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)

    # QtQuick/Shapes and QtQuick/NativeStyle are already here from previous step
    set(QML_MODULES QtQuick/Layouts QtQuick/Controls QtQuick/Templates QtQuick/Window QtQuick/Shapes QtQuick/NativeStyle)
    foreach(QML_MOD ${QML_MODULES})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/qml/${QML_MOD}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${QT_INSTALL_PATH}/qml/${QML_MOD}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/qml/${QML_MOD}")
    endforeach()

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
