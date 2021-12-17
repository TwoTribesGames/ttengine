cmake_minimum_required(VERSION 3.2)
Project(RIVE)

string(TIMESTAMP BUNDLE_VERSION "%y%m%d%H" UTC)

find_path(TTDEV_ROOT libs/CMakeLists.txt
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ttdev ENV TTDEV_ROOT
    DOC "The location of the TTDEV root directory"
    NO_DEFAULT_PATH
)
if (NOT TTDEV_ROOT)
    message(FATAL_ERROR "Could not find TTDEV_ROOT make sure to set the ENV var or set the CMake TTDEV_ROOT variable")
endif()
add_subdirectory(${TTDEV_ROOT}/libs ttdev)

OPTION(DEMO_BUILD "Enable the demo build" OFF)

## Setup rule to generate the revision header
## Output destination is in the ${PROJECT_NAME}_REVISION_HEADER_FILE variable
GenerateRevisionHeader(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/toki
    ${TTDEV_REVISION_TEMPLATE}
    ${PROJECT_NAME}__REVISION_HEADER_FILE
)

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
            osx_sdl2/src/**
        FILES
            ../assets/source/mac/icons/app_icon.icns
    )
    set_source_files_properties(
        ../assets/source/mac/icons/app_icon.icns
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
    ../assets/source/shared/scripts
FILES
    ${${PROJECT_NAME}__REVISION_HEADER_FILE}
INCLUDES
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
LINK
    ${PROJECT_NAME}_shared
PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/osx_sdl2/res/Info.plist.in"
    MACOSX_BUNDLE_COPYRIGHT "Copyright © 2017 Two Tribes Publishing B.V."
    MACOSX_BUNDLE_ICON_FILE "app_icon.icns"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.twotribes.rive"
    MACOSX_BUNDLE_BUNDLE_NAME "RIVE"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.15"
    MACOSX_BUNDLE_BUNDLE_VERSION "${BUNDLE_VERSION}"
    FOLDER Game
)

CopyDependentLibs(${PROJECT_NAME})
