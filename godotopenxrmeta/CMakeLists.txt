cmake_minimum_required(VERSION 3.22.1)

## Common dependencies
include(${CMAKE_SOURCE_DIR}/../common/CMakeLists.txt)


## Project definition
project(godotopenxrvendors LANGUAGES CXX)

## OpenXR Mobile loader library
# Sets the path to the OpenXR mobile library directory.
set(OPENXR_MOBILE_ROOT_DIR "${CMAKE_SOURCE_DIR}/../thirdparty/ovr_openxr_mobile_sdk/OpenXR")
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
        ${COMMON_LIB_SOURCES}
        ${COMMON_LIB_HEADERS}
        )

target_include_directories(${CMAKE_PROJECT_NAME}
        SYSTEM PUBLIC
        ${GODOT_CPP_INCLUDE_DIRECTORIES}
        ${OPENXR_HEADERS_DIR}
        ${OPENXR_MOBILE_HEADERS_DIR}
        ${COMMON_LIB_HEADERS_DIR}
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

add_definitions(-DMETA_VENDOR_ENABLED)
