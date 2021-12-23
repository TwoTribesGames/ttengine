cmake_minimum_required(VERSION 3.2)
Project(Game)

string(TIMESTAMP BUNDLE_VERSION "%y%m%d%H" UTC)

OPTION(DEMO_BUILD "Enable the demo build" OFF)

CreateLibrary(${PROJECT_NAME}_shared
DIRS
    shared/src/toki/**
    shared/inc/toki/**
INCLUDES
    PUBLIC
        shared/inc
LINK
    tt_shared recastnavigation
    $<$<BOOL:${STEAM_BUILD}>:tt_steam>
DEFINES
    PUBLIC
        TT_DEMO_BUILD=$<BOOL:${DEMO_BUILD}>
PROPERTIES
	FOLDER Game
)

if(LINUX)
    set(PLATFORM_SOURCE
        DIRS
            linux/src/**
    )
elseif(APPLE)
    set(PLATFORM_SOURCE
        DIRS
            osx/src/**
        FILES
            ${TTDEV_ROOT}/assets/source/mac/icons/app_icon.icns
    )
    set_source_files_properties(
            ${TTDEV_ROOT}/assets/source/mac/icons/app_icon.icns
        PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources
    )
elseif(WIN32)
    set(PLATFORM_SOURCE
        DIRS
	    windows_sdl2/src/**
    )
else()
    message(FATAL_ERROR "Platform main source not defined")
endif()

CreateProgram(${PROJECT_NAME}
    ${PLATFORM_SOURCE}
DIRS
    sdl2/src/**
REFERENCE
    ${TTDEV_ROOT}/assets/source/shared/scripts
FILES
    ${${PROJECT_NAME}__REVISION_HEADER_FILE}
INCLUDES
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
LINK
    ${PROJECT_NAME}_shared
PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/osx/res/Info.plist.in"
    MACOSX_BUNDLE_COPYRIGHT "Copyright ©"
    MACOSX_BUNDLE_ICON_FILE "app_icon.icns"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.game"
    MACOSX_BUNDLE_BUNDLE_NAME "Game"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    MACOSX_BUNDLE_BUNDLE_VERSION "${BUNDLE_VERSION}"
    FOLDER Game
)

CopyDependentLibs(${PROJECT_NAME})
