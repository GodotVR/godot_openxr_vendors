cmake_minimum_required(VERSION 3.22.1)

## Project definition
project(godotopenxrvendors LANGUAGES CXX)

## Common dependencies
include(${PROJECT_SOURCE_DIR}/../common/common.cmake)

## lynx OpenXR loader library
set(LYNX_OPENXR_LIB_PATH "${PROJECT_SOURCE_DIR}/../thirdparty/lynx_openxr_sdk/${ANDROID_ABI}/libopenxr_loader.so")
add_library(openxr_loader
        SHARED
        IMPORTED GLOBAL
        )
set_target_properties(openxr_loader PROPERTIES IMPORTED_LOCATION ${LYNX_OPENXR_LIB_PATH})

## Setup the project sources
file(GLOB_RECURSE ANDROID_SOURCES ${PROJECT_SOURCE_DIR}/src/main/cpp/*.c**)
file(GLOB_RECURSE ANDROID_HEADERS ${PROJECT_SOURCE_DIR}/src/main/cpp/*.h**)

add_library(${PROJECT_NAME}
        SHARED
        ${ANDROID_SOURCES}
        ${ANDROID_HEADERS}
        ${COMMON_LIB_SOURCES}
        ${COMMON_LIB_HEADERS}
        )

target_include_directories(${PROJECT_NAME}
        SYSTEM PUBLIC
        ${GODOT_CPP_INCLUDE_DIRECTORIES}
        ${OPENXR_HEADERS_DIR}
        ${COMMON_LIB_HEADERS_DIR}
        )

target_link_libraries(${PROJECT_NAME}
        android
        log
        ${GODOT-CPP}
        openxr_loader
        )

# Add the compile flags
set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS ${GODOT_COMPILE_FLAGS})
set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS ${GODOT_LINKER_FLAGS})

add_definitions(-DLYNX_VENDOR_ENABLED)
