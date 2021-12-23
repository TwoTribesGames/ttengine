cmake_minimum_required(VERSION 3.2)
Project(ttdev)

### pull in boilerplate cmake
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../tools/cmake")
include(BoilerPlate)
include(UtilityFunctions)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(APPLE)
    set(PLATFORM_PREFIX "osx")
    set(PLATFORM_LIB_PREFIX "osx")
    set(USE_OPENAL_SOFT ON)
    set(USE_SDL2 ON)
    set(POSIX TRUE)
elseif(LINUX)
    set(PLATFORM_PREFIX "linux")
    set(PLATFORM_LIB_PREFIX "linux")
    set(USE_OPENAL_SOFT ON)
    set(USE_SDL2 ON)
    set(POSIX TRUE)
elseif(WIN32)
    set(PLATFORM_PREFIX "windows_sdl2")
    set(PLATFORM_LIB_PREFIX "windows")
	set(USE_OPENAL_SOFT ON)
    set(USE_SDL2 ON)
else()
    message(FATAL_ERROR "Unhandled platform")
endif()

### find utility programs
find_program(TTDEV_UPDATE_REV_INFO_SH
    NAMES update_revision_info.sh updaterevision.bat
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../tools/${PLATFORM_LIB_PREFIX}/revision_info/
    NO_DEFAULT_PATH
)
find_file(TTDEV_LIB_REVISION_TEMPLATE __revision_template_libs.h
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../tools/${PLATFORM_LIB_PREFIX}/revision_info/
    NO_DEFAULT_PATH
)
find_file(TTDEV_REVISION_TEMPLATE __revision_template.h
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../tools/${PLATFORM_LIB_PREFIX}/revision_info/
    NO_DEFAULT_PATH
)

find_path(TTDEV_PREBUILT_PLATFORM_ROOT prebuilt
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_LIB_PREFIX}
    NO_DEFAULT_PATH
)
mark_as_advanced(
    TTDEV_UPDATE_REV_INFO_SH   
    TTDEV_LIB_REVISION_TEMPLATE
    TTDEV_REVISION_TEMPLATE
    TTDEV_PREBUILT_PLATFORM_ROOT
)

include(TTSDKFunctions)

### Find Libraries
find_package(OpenGL)
find_package(Threads)
if(WIN32)
    FindPrebuiltLibrary(OPENAL_LIBRARY openal32 openal)
    FindPrebuiltInclude(OPENAL_INCLUDES openal AL/al.h)
    if (USE_SDL2)
        FindPrebuiltLibrary(SDL2_LIBRARY_lib SDL2 sdl/x64)
        FindPrebuiltLibrary(SDL2_LIBRARY_main SDL2main sdl/x64)
        set(SDL2_LIBRARY ${SDL2_LIBRARY_lib} ${SDL2_LIBRARY_main})
    endif()
    FindPrebuiltLibrary(STEAM_LIBRARY steam_api6644 steam)
else()
    if(USE_OPENAL_SOFT)
        FindPrebuiltLibrary(OPENAL_LIBRARY openal)
        FindPrebuiltInclude(OPENAL_INCLUDES openal AL/al.h)
        if(NOT OPENAL_INCLUDES)
            set(OPENAL_INCLUDES OFF)
        endif()
    else()
        find_package(OpenAL)
    endif()
    if (USE_SDL2)
        FindPrebuiltLibrary(SDL2_LIBRARY SDL2)
    endif()
    FindPrebuiltLibrary(STEAM_LIBRARY steam_api steam)
endif()

if(LINUX OR APPLE)
    FindPrebuiltLibrary(CURL_LIBRARY curl)
    FindPrebuiltInclude(CURL_INCLUDES curl curl/curl.h)
    list(APPEND TT_SYS_PLATFORM_INCLUDES ${CURL_INCLUDES})
    list(APPEND TT_SYS_PLATFORM_LIBRARIES ${CURL_LIBRARY} Threads::Threads)
    if(APPLE)
        find_library(SECURITY_LIBRARY Security)
        list(APPEND TT_SYS_PLATFORM_LIBRARIES ${SECURITY_LIBRARY})
    endif()
endif()

if (APPLE)
    find_library(APPKIT_LIBRARY AppKit)
    find_library(SYSTEM_CONFIGURATION_LIBRARY SystemConfiguration)
    find_library(FOUNDATION_LIBRARY Foundation)

    list(APPEND TT_SYS_PLATFORM_LIBRARIES ${FOUNDATION_LIBRARY} ${SYSTEM_CONFIGURATION_LIBRARY} ${APPKIT_LIBRARY})
endif()

if (WIN32)
    list(APPEND TT_SYS_PLATFORM_LIBRARIES winhttp)
endif()

### Setup Compler definitions

set(EXTRA_BASE_CFLAGS
#    -Wno-missing-field-initializers
#    -Wno-ignored-qualifiers
#    -Wno-missing-braces # Seems to be incorrectly triggered by OpenALSoundSystem
    -Wno-absolute-value
)

if(LINUX)
    list(APPEND EXTRA_BASE_CFLAGS -mpc64 -mfpmath=sse)
endif()
CheckCXXFlags(TT_BASE_CFLAGS -Wno-unused-local-typedef ${EXTRA_BASE_CFLAGS})

CreateInterface(tt_base
    INCLUDES
        INTERFACE
            ${PLATFORM_PREFIX}/inc
            $<$<BOOL:${POSIX}>:${CMAKE_CURRENT_SOURCE_DIR}/posix/inc>
            $<$<BOOL:${USE_SDL2}>:${CMAKE_CURRENT_SOURCE_DIR}/sdl2/inc>
            shared/inc
    FEATURES
        INTERFACE
            cxx_auto_type
    FLAGS
        INTERFACE
            ${TT_BASE_CFLAGS}
    DEFINES
        INTERFACE
            $<$<BOOL:${LINUX}>:TT_PLATFORM_LNX>
            $<$<BOOL:${WIN32}>:TT_PLATFORM_WIN;_UNICODE;_CRT_SECURE_NO_WARNINGS>
            $<$<BOOL:${APPLE}>:TT_PLATFORM_OSX;TT_PLATFORM_OSX_MAC>
            $<$<BOOL:${USE_SDL2}>:TT_PLATFORM_SDL>
            $<$<CONFIG:Debug>:TT_BUILD_DEV;_DEBUG;TT_ASSERT_ON>
            $<$<NOT:$<CONFIG:Debug>>:TT_BUILD_FINAL>
)

## Setup all the libs

CreateLibrary(ogg
DIRS
    shared/src/ogg
    shared/inc/ogg
LINK
    tt_base
PROPERTIES
    FOLDER 3rdParty
)

CreateLibrary(vorbis
DIRS
    shared/src/vorbis
    shared/inc/vorbis
LINK
    ogg
PROPERTIES
    FOLDER 3rdParty
)

CreateLibrary(z
DIRS
    shared/src/libz
    shared/inc/libz
LINK
    tt_base
PROPERTIES
    FOLDER 3rdParty
)

CreateLibrary(png
DIRS
    shared/src/libpng
    shared/inc/libpng
LINK
    z
PROPERTIES
    FOLDER 3rdParty
)

CreateLibrary(json
DIRS
    shared/src/json
    shared/inc/json
LINK
    tt_base
PROPERTIES
    FOLDER 3rdParty
)

CreateLibrary(squirrel
DIRS
    shared/src/squirrel/**
    shared/inc/squirrel/**
LINK
    tt_base
PROPERTIES
    FOLDER 3rdParty
)

CreateLibrary(spark
DIRS
    shared/inc/spark/**
    shared/src/spark/**
EXCLUDE
    shared/src/spark/SPK*_All.cpp
    shared/src/spark/SPK_*_ALL.cpp
LINK
    tt_base
PROPERTIES
    FOLDER 3rdParty
)

set(VISUAL_BOY_COMMON
    LINK tt_base
    PROPERTIES FOLDER 3rdParty
    DEFINES PRIVATE C_CORE NO_LINK NO_PNG register=
)

CreateLibrary(VisualBoy_fex
DIRS
    shared/inc/visualboy/lib/fex
    shared/src/visualboy/lib/fex
EXCLUDE
    shared/inc/visualboy/lib/fex/Zip7_Extractor.h
    shared/src/visualboy/lib/fex/Zip7_Extractor.cpp
INCLUDES
    PRIVATE
        shared/inc/visualboy/lib
        shared/inc/visualboy/lib/fex
${VISUAL_BOY_COMMON}
)

CreateLibrary(VisualBoy
DIRS
    shared/inc/visualboy
    shared/src/visualboy
    shared/src/visualboy/lib
    shared/src/visualboy/lib
    shared/inc/visualboy/lib/apu
    shared/src/visualboy/lib/apu
    shared/inc/visualboy/lib/common
    shared/src/visualboy/lib/common
    shared/inc/visualboy/lib/gb
    shared/src/visualboy/lib/gb
    shared/inc/visualboy/lib/gba
    shared/src/visualboy/lib/gba
    shared/inc/visualboy/lib/filters
    shared/src/visualboy/lib/filters
    shared/inc/visualboy/lib/filters/hq
    shared/src/visualboy/lib/filters/hq
    shared/inc/visualboy/lib/filters/xBRZ
    shared/src/visualboy/lib/filters/xBRZ
FILES
    shared/inc/visualboy/lib/sdl/filters.h
    shared/src/visualboy/lib/sdl/filters.cpp
INCLUDES
    PRIVATE
        shared/inc/visualboy/lib
        shared/inc/visualboy/lib/apu
        shared/inc/visualboy/lib/common
        shared/inc/visualboy/lib/gb
        shared/inc/visualboy/lib/gba
        shared/inc/visualboy/lib/filters
        shared/inc/visualboy/lib/filters/hq
        shared/inc/visualboy/lib/filters/xBRZ
        shared/inc/visualboy/lib/sdl
EXCLUDE
    shared/inc/visualboy/lib/common/ffmpeg.h
    shared/src/visualboy/lib/common/ffmpeg.cpp
    shared/inc/visualboy/lib/common/SoundSDL.h
    shared/src/visualboy/lib/common/SoundSDL.cpp
    shared/src/visualboy/lib/gb/GBASockClient.cpp
    shared/src/visualboy/lib/gba/GBASockClient.cpp
    shared/src/visualboy/lib/gba/GBALink.cpp
    shared/src/visualboy/lib/gba/remote.cpp
LINK
    VisualBoy_fex
${VISUAL_BOY_COMMON}
)

CreateLibrary(gwen
DIRS
    shared/src/gwen/**
    shared/inc/gwen/**
EXCLUDE
    shared/src/gwen/Platforms/**
    shared/src/gwen/Renderers/**
FILES
    shared/src/gwen/Platforms/SDL.cpp
    shared/src/gwen/Renderers/TTRenderer.cpp
    shared/src/gwen/Renderers/DebugFont/FontData.h
LINK
    tt_base
PROPERTIES
    FOLDER 3rdParty
)

CreateLibrary(recastnavigation
DIRS
    shared/src/recastnavigation/DebugUtils/**
    shared/src/recastnavigation/Detour
    shared/src/recastnavigation/DetourCrowd
    shared/src/recastnavigation/DetourTileCache
    shared/src/recastnavigation/Recast
    shared/inc/recastnavigation/**
LINK
    tt_engine
PROPERTIES
    FOLDER 3rdParty
)

CreateLibrary(tt_engine
DIRS
    shared/src/tt/engine/**
    shared/inc/tt/engine/**
EXCLUDE
    shared/src/tt/engine/renderer/FixedFunctionHardware_stub.cpp
LINK
    tt_shared tt_math spark
PROPERTIES
    FOLDER TwoTribes
)

CreateLibrary(tt_math
DIRS
    shared/src/tt/math/**
    shared/inc/tt/math/**
LINK
    tt_shared
PROPERTIES
    FOLDER TwoTribes
)

CreateLibrary(tt_shared
DIRS
    shared/src/tt/algorithms
    shared/inc/tt/algorithms
    shared/src/tt/app
    shared/inc/tt/app
    shared/src/tt/args
    shared/inc/tt/args
    shared/src/tt/asset
    shared/inc/tt/asset
    shared/src/tt/audio/**
    shared/inc/tt/audio/**
    shared/src/tt/cfg
    shared/inc/tt/cfg
    shared/src/tt/code
    shared/inc/tt/code
    shared/src/tt/com
    shared/inc/tt/com
    shared/src/tt/compression/**
    shared/inc/tt/compression/**
    shared/src/tt/doc/**
    shared/inc/tt/doc/**
    shared/src/tt/error
    shared/inc/tt/errir
    shared/src/tt/fileformat/**
    shared/inc/tt/fileformat/**
    shared/src/tt/fs/**
    shared/inc/tt/fs/**
    shared/src/tt/fs/utils
    shared/src/tt/gwen
    shared/inc/tt/gwen
    shared/src/tt/http
    shared/inc/tt/http
    shared/src/tt/input
    shared/inc/tt/input
    shared/src/tt/loc
    shared/inc/tt/loc
    shared/src/tt/log
    shared/inc/tt/log
    shared/src/tt/menu/**
    shared/inc/tt/menu/**
    shared/src/tt/particles
    shared/inc/tt/particls
    shared/src/tt/pres/**
    shared/inc/tt/pres/**
    shared/inc/tt/privacy
    shared/src/tt/profiler
    shared/inc/tt/profiler
    shared/inc/tt/score
    shared/src/tt/script/**
    shared/inc/tt/script/**
    shared/src/tt/settings
    shared/inc/tt/settings
    shared/src/tt/snd
    shared/inc/tt/snd
    shared/src/tt/stats
    shared/inc/tt/stats
    shared/src/tt/str
    shared/inc/tt/str
    shared/src/tt/streams
    shared/inc/tt/streams
    shared/src/tt/system
    shared/inc/tt/system
    shared/src/tt/undo
    shared/inc/tt/undo
    shared/src/tt/version
    shared/inc/tt/version
    shared/src/tt/xml/**
    shared/inc/tt/xml/**
FILES
    shared/src/tt/thread/ThreadedWorkload.cpp
    shared/inc/tt/thread/ThreadedWorkload.h
INCLUDES
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC
        $<$<BOOL:${OPENAL_INCLUDES}>:${OPENAL_INCLUDES}>
DEFINES
    PUBLIC
        $<$<BOOL:${USE_OPENAL_SOFT}>:TT_FORCE_OPENAL_SOFT>
        $<$<BOOL:${USE_SDL2}>:USE_LODEPNG>
FILES
    ${TTDEV_REVISION_HEADER_FILE}
LINK
    tt_base
    gwen json png vorbis squirrel VisualBoy
    tt_engine tt_sys
    ${OPENAL_LIBRARY}
    $<$<BOOL:${USE_SDL2}>:lodepng>
PROPERTIES
    FOLDER TwoTribes
)

OPTION(STEAM_BUILD "Enable a steam build" OFF)

CreateLibrary(tt_steam
DIRS
    shared/src/tt/steam
    shared/inc/tt/steam
DEFINES
    INTERFACE
        TT_STEAM_BUILD
LINK
    tt_shared
    ${STEAM_LIBRARY}
PROPERTIES
    FOLDER TwoTribes
)

CreateLibrary(glloadgen
DIRS
    shared/inc/glloadgen
    shared/src/glloadgen
INCLUDES
    shared/inc
DEFINES
    PRIVATE
        $<$<BOOL:${USE_SDL2}>:__SDL2__>
PROPERTIES
    FOLDER 3rdParty
)

if (USE_SDL2)
CreateLibrary(lodepng
DIRS
    shared/inc/lodepng
    shared/src/lodepng
INCLUDES
    PRIVATE
        shared/inc/lodepng
        shared/inc
PROPERTIES
    FOLDER 3rdParty
)

CreateLibrary(tt_sys_sdl2
DIRS
    sdl2/src/tt/**
    sdl2/inc/tt/**
LINK
    tt_shared
    ${SDL2_LIBRARY}
    glloadgen
    lodepng
PROPERTIES
    FOLDER TwoTribes
)
endif()

if(POSIX)
CreateLibrary(tt_sys_posix
DIRS
    posix/src/tt/**
    posix/inc/tt/**
LINK
    tt_shared
PROPERTIES
    FOLDER TwoTribes
)
endif()

CreateLibrary(tt_sys
DIRS
    ${PLATFORM_PREFIX}/src/tt/**
    ${PLATFORM_PREFIX}/inc/tt/**
INCLUDES
    ${TT_SYS_PLATFORM_INCLUDES}
LINK
    $<$<BOOL:${USE_SDL2}>:tt_sys_sdl2>
    $<$<BOOL:${POSIX}>:tt_sys_posix>
    ${TT_SYS_PLATFORM_LIBRARIES}
PROPERTIES
    FOLDER TwoTribes
)
