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


project(common LANGUAGES CXX)

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
