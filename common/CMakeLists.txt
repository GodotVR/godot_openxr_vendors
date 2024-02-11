cmake_minimum_required(VERSION 3.22.1)


## Default configs
# Default android abi is arm64-v8a
if (NOT ANDROID_ABI)
    set(ANDROID_ABI "arm64-v8a")
endif (NOT ANDROID_ABI)

if (ANDROID_ABI STREQUAL "armeabi-v7a")
    set(GODOT_CPP_LIB_ABI "arm32")
elseif (ANDROID_ABI STREQUAL "x86")
    set(GODOT_CPP_LIB_ABI "x86_32")
elseif (ANDROID_ABI STREQUAL "x86_64")
    set(GODOT_CPP_LIB_ABI "x86_64")
else ()
    set(GODOT_CPP_LIB_ABI "arm64")
endif ()

# Default android platform is android-21
if (NOT ANDROID_PLATFORM)
    set(ANDROID_PLATFORM "android-21")
endif (NOT ANDROID_PLATFORM)

if (NOT (ANDROID_STL STREQUAL "c++_shared"))
    set(ANDROID_STL "c++_shared")
endif (NOT (ANDROID_STL STREQUAL "c++_shared"))

# Default build type is Debug
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-D_DEBUG)
    set(GODOT_CPP_LIB_BUILD_TYPE debug)
    set(OPENXR_MOBILE_LIB_BUILD_TYPE Debug)
else ()
    add_definitions(-DNDEBUG)
    set(GODOT_CPP_LIB_BUILD_TYPE release)
    set(OPENXR_MOBILE_LIB_BUILD_TYPE Release)
endif (CMAKE_BUILD_TYPE MATCHES Debug)

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


project(common LANGUAGES CXX)

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

## godot-cpp library
set(GODOT_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/godot-cpp")
set(GODOT-CPP "godot-cpp")

# Use the godot-cpp prebuilt static binary
set(GODOT_CPP_STATIC_LIB "${GODOT_CPP_DIR}/bin/libgodot-cpp.android.template_${GODOT_CPP_LIB_BUILD_TYPE}.${GODOT_CPP_LIB_ABI}.a")

list(APPEND GODOT_CPP_INCLUDE_DIRECTORIES "${GODOT_CPP_DIR}/include")
list(APPEND GODOT_CPP_INCLUDE_DIRECTORIES "${GODOT_CPP_DIR}/gen/include")
list(APPEND GODOT_CPP_INCLUDE_DIRECTORIES "${GODOT_CPP_DIR}/gdextension")

add_library(${GODOT-CPP}
        STATIC
        IMPORTED GLOBAL
        INTERFACE_INCLUDE_DIRECTORIES "${GODOT_CPP_INCLUDE_DIRECTORIES}"
        )
set_target_properties(${GODOT-CPP} PROPERTIES IMPORTED_LOCATION ${GODOT_CPP_STATIC_LIB})


## OpenXR headers
set(OPENXR_HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/openxr/include")


# Common lib
set(COMMON_LIB_HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/main/cpp/include)

file(GLOB_RECURSE COMMON_LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/main/cpp/*.c**)
file(GLOB_RECURSE COMMON_LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/main/cpp/*.h**)
