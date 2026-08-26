set(QT_NO_EMSCRIPTEN_VERSION_CHECK ON CACHE BOOL "" FORCE)
cmake_minimum_required(VERSION 3.16)
project(taskHelper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (NOT DEFINED CMAKE_PREFIX_PATH)
    if (WIN32 AND NOT EMSCRIPTEN)
        set(CMAKE_PREFIX_PATH "C:/Qt/6.11.0/msvc2022_64")
    endif ()
endif ()
if (APPLE)
    if (DEFINED ENV{QT_PATH})
        set(CMAKE_PREFIX_PATH "$ENV{QT_PATH}")
    else()
        set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.11.0/macos")
    endif()
endif ()

include(FetchContent)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force static libraries for WASM" FORCE)

FetchContent_Declare(
        md4c
        GIT_REPOSITORY https://github.com/mity/md4c.git
        GIT_TAG        release-0.5.2  # Changed from v0.5.2
)

FetchContent_MakeAvailable(md4c)

set_target_properties(md4c PROPERTIES
        AUTOMOC OFF
        AUTOUIC OFF
        AUTORCC OFF
)
set_target_properties(md4c-html PROPERTIES
        AUTOMOC OFF
        AUTOUIC OFF
        AUTORCC OFF
)

set_target_properties(md2html PROPERTIES
        AUTOMOC OFF
        AUTOUIC OFF
        AUTORCC OFF
)

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
        #src/UI/componets/Side_Bar/Components/CoreNavigationSection.cpp
        src/UI/componets/Side_Bar/Components/WorkspaceContextSection.cpp
        src/UI/componets/Side_Bar/Components/ProjectsSection.cpp
        src/UI/componets/Side_Bar/Components/FiltersSection.cpp
        src/md4c/md4c.cpp
        src/UI/componets/Content_Windows/MainContentView.cpp
        src/helpers/AppStateController.cpp
        src/Data/workspace/WorkspaceRepository.cpp
        src/UI/componets/Content_Windows/WorkspaceView.cpp
        src/Data/workspace/WorkspaceSwitchDialog.cpp
        src/Data/workspace/WorkspaceCreateDialog.cpp
        src/Data/workspace/WorkspaceDeleteDialog.cpp
        src/Data/workspace/WorkspaceSettingsWindow.cpp
        src/UI/componets/Content_Windows/Page/NetworkTestPage.cpp
        src/Data/workspace/Manager/TaskManager.cpp
        src/Data/workspace/Manager/NoteManager.cpp
        src/UI/componets/Content_Windows/Page/wsHomePage.cpp
        src/Data/Project/ProjectCreateDialog.cpp
        src/UI/componets/Dialog/DropAreaWidget.cpp
        src/UI/componets/Dialog/UploadDialog.cpp
        src/Data/workspace/Manager/TabManager.cpp
        src/UI/componets/Top_Menubar/TabBar.cpp
        src/UI/componets/Content_Windows/Page/wsNotePage.cpp
        src/UI/componets/Side_Bar/nu_componets/CoreNavigationSection.cpp
        src/UI/componets/Side_Bar/nu_componets/FooterSection.cpp
        src/UI/componets/Side_Bar/nu_componets/ProjectsSection.cpp)

set (RESOURCE_FILES
        resources/resources.qrc)

set (HEADER_FILES
        include/UI/components/Top_Menubar/topMenubar.h
        include/UI/components/Top_Menubar/windowsAction.h
        include/UI/components/Top_Menubar/NavigationBar.h
        include/UI/components/Top_Menubar/infoBar.h
        include/UI/mainWIndow.h
        include/MD4C/mdAPI.h
        include/UI/components/Top_Menubar/MenuButtonBar.h
        include/UI/components/SIde_Bar/sideBar.h
        #include/UI/components/SIde_Bar/componets/FooterSection.h
        include/UI/components/SIde_Bar/componets/FiltersSection.h
        include/UI/components/SIde_Bar/componets/ProjectsSection.h
        #include/UI/components/SIde_Bar/componets/CoreNavigationSection.h
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
        include/UI/components/Content_Windows/page/loginPage.h
        include/UI/components/Content_Windows/page/wsHomePage.h
        include/Data/workspace/Manager/TaskManager.h
        include/Data/workspace/Manager/NoteManager.h
        include/Data/Networking/SyncManager.h
        include/Data/Project/ProjectCreateDialog.h
        include/Data/attachments/AttachmentService.h
        include/Data/attachments/AttachmentValidator.h
        include/helpers/AttachmentManager.h
        include/UI/components/Dialogs/UploadDialog.h
        include/UI/components/Dialogs/DropAreaWidget.h
        include/UI/components/Content_Windows/page/wsNotePage.h
        include/UI/components/Top_Menubar/TabBar.h
        include/Data/workspace/Manager/TabManager.h
        include/UI/components/SIde_Bar/nu_componets/BaseNavTree.h
        include/UI/components/SIde_Bar/nu_componets/CoreNavigationSection.h
        include/UI/components/SIde_Bar/nu_componets/FooterSection.h

        include/UI/components/SIde_Bar/SideBarMode.h
        include/UI/components/SIde_Bar/nu_componets/ProjectSection.h
        include/UI/components/Content_Windows/page/wsProjectPage.h
        include/UI/components/Content_Windows/page/wsNoteListPage.h
        include/UI/components/Content_Windows/page/wsTaskBoardPage.h)

qt_add_executable(taskHelper ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES})

target_include_directories(taskHelper PRIVATE
        ${md4c_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

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
        md4c
        md4c-html
)

if (WIN32 AND NOT EMSCRIPTEN)
    target_link_libraries(taskHelper PRIVATE Dwmapi)
    target_compile_definitions(taskHelper PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)
endif()

if (EMSCRIPTEN)
    set(QT_EMSDK_PATH "C:/emsdk" CACHE PATH "" FORCE)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.11.0/wasm_multithread")
    set_target_properties(taskHelper PROPERTIES 
        SUFFIX ".js"
        LINK_FLAGS "-sALLOW_MEMORY_GROWTH=1 -pthread"
    )
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

# --- NEW DEPLOYMENT LOGIC FOR WINDOWS ---
if (WIN32)
    # Locate the windeployqt tool provided by your Qt installation
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${CMAKE_PREFIX_PATH}/bin")

    if(WINDEPLOYQT_EXECUTABLE)
        # Run windeployqt automatically after building the executable
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --qmldir "${CMAKE_CURRENT_SOURCE_DIR}"
                --no-translations
                --no-compiler-runtime
                "$<TARGET_FILE:${PROJECT_NAME}>"
                COMMENT "Running windeployqt to automatically gather required DLLs and QML plugins..."
        )
    else()
        message(WARNING "windeployqt not found! DLLs will not be copied automatically.")
    endif()
endif ()