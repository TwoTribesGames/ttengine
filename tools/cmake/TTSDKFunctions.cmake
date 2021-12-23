cmake_minimum_required(VERSION 2.8.12)

## Find a prebuiltlib
function(FindPrebuiltLibrary result_var libname)
    if(ARGV2)
        set(name ${ARGV2})
    else()
        set(name ${libname})
    endif()
    # check prebuilt directory first
    find_library(${result_var}
        NAMES ${libname}
        PATHS ${TTDEV_PREBUILT_PLATFORM_ROOT}/prebuilt/${name}/lib ${TTDEV_PREBUILT_PLATFORM_ROOT}/prebuilt/${name}
        NO_DEFAULT_PATH
    )
    # Check system dir
    find_library(${result_var}
        NAMES ${libname})
    if(NOT ${result_var})
        message(FATAL_ERROR "Could not find library ${libname} in prebuilt folder ${name}")
    endif()
endfunction()

function(FindPrebuiltInclude result_var name incname)
    # check prebuilt directory first
    find_path(${result_var}
        NAMES ${incname}
        PATHS ${TTDEV_PREBUILT_PLATFORM_ROOT}/prebuilt/${name}/include
        NO_DEFAULT_PATH
    )
endfunction()
