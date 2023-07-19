cmake_minimum_required(VERSION 3.22.1)

## Default configs
# Default android abi is arm64-v8a
if (NOT ANDROID_ABI)
    set(ANDROID_ABI "arm64-v8a")
endif (NOT ANDROID_ABI)

# Default android platform is android-21
if (NOT ANDROID_PLATFORM)
    set(ANDROID_PLATFORM "android-21")
endif (NOT ANDROID_PLATFORM)

# Default build type is Debug
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)

if (NOT (ANDROID_STL STREQUAL "c++_shared"))
    set(ANDROID_STL "c++_shared")
endif (NOT (ANDROID_STL STREQUAL "c++_shared"))

# Check if ANDROID_NDK is set.
if (NOT ANDROID_NDK)
    # Set to ANDROID_NDK_HOME environment variable if it's set.
    if (DEFINED ENV{ANDROID_NDK_HOME})
        set(ANDROID_NDK $ENV{ANDROID_NDK_HOME})
    else (DEFINED ENV{ANDROID_NDK_HOME})
        message(WARNING "ANDROID_NDK_HOME is not set")
    endif (DEFINED ENV{ANDROID_NDK_HOME})
endif (NOT ANDROID_NDK)

# Check if CMAKE_TOOLCHAIN_FILE is set.
if (NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
endif (NOT CMAKE_TOOLCHAIN_FILE)

if (NOT DEFINED BITS)
    set(BITS 32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(BITS 64)
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
endif (NOT DEFINED BITS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


## Common dependencies
include(${CMAKE_SOURCE_DIR}/../common/CMakeLists.txt)


## Project definition
project(godotopenxrmeta LANGUAGES CXX)

add_definitions(-DANDROID)

set(GODOT_COMPILE_FLAGS)
set(GODOT_LINKER_FLAGS)

set(GODOT_LINKER_FLAGS "-Wl")

set(GODOT_COMPILE_FLAGS "-fPIC -g -Wwrite-strings")
set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wchar-subscripts -Wcomment -Wdisabled-optimization")
set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wformat -Wformat=2 -Wformat-security -Wformat-y2k")
set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wimport -Winit-self -Winline -Winvalid-pch")
set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wlong-long -Wmissing-braces -Wmissing-format-attribute")
set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wpointer-arith")
set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wredundant-decls -Wreturn-type -Wsequence-point")
set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wswitch -Wswitch-enum -Wtrigraphs")
set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wuninitialized -Wunknown-pragmas -Wunreachable-code -Wunused-label")
set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wunused-value -Wvariadic-macros -Wvolatile-register-var -Wno-error=attributes")

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wno-ignored-attributes")
endif ()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -fno-omit-frame-pointer -O0")
else ()
    set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -O3")
endif (CMAKE_BUILD_TYPE MATCHES Debug)


## OpenXR Mobile loader library
# Sets the path to the OpenXR mobile library directory.
set(OPENXR_MOBILE_ROOT_DIR "${CMAKE_SOURCE_DIR}/libs/ovr_openxr_mobile_sdk/OpenXR")
set(OPENXR_MOBILE_HEADERS_DIR "${OPENXR_MOBILE_ROOT_DIR}/Include" CACHE STRING "")

set(OPENXR_MOBILE_LIB_PATH "${OPENXR_MOBILE_ROOT_DIR}/Libs/Android/${ANDROID_ABI}/${OPENXR_MOBILE_LIB_BUILD_TYPE}/libopenxr_loader.so")
add_library(openxr_loader
        SHARED
        IMPORTED GLOBAL
        )
set_target_properties(openxr_loader PROPERTIES IMPORTED_LOCATION ${OPENXR_MOBILE_LIB_PATH})


## Setup the project sources
file(GLOB_RECURSE ANDROID_SOURCES ${CMAKE_SOURCE_DIR}/src/main/cpp/*.c**)
file(GLOB_RECURSE ANDROID_HEADERS ${CMAKE_SOURCE_DIR}/src/main/cpp/*.h**)

add_library(${CMAKE_PROJECT_NAME}
        SHARED
        ${ANDROID_SOURCES}
        ${ANDROID_HEADERS}
        )

target_include_directories(${CMAKE_PROJECT_NAME}
        SYSTEM PUBLIC
        ${GODOT_CPP_INCLUDE_DIRECTORIES}
        ${OPENXR_HEADERS_DIR}
        ${OPENXR_MOBILE_HEADERS_DIR}
        )

target_link_libraries(${CMAKE_PROJECT_NAME}
        android
        log
        ${GODOT-CPP}
        openxr_loader
        )

# Add the compile flags
set_property(TARGET ${CMAKE_PROJECT_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS ${GODOT_COMPILE_FLAGS})
set_property(TARGET ${CMAKE_PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS ${GODOT_LINKER_FLAGS})
