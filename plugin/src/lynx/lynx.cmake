cmake_minimum_required(VERSION 3.22.1)

## lynx OpenXR loader library
set(LYNX_OPENXR_LIB_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../thirdparty/lynx_openxr_sdk/${ANDROID_ABI}/libopenxr_loader.so")
add_library(openxr_loader
        SHARED
        IMPORTED GLOBAL
)
set_target_properties(openxr_loader PROPERTIES IMPORTED_LOCATION ${LYNX_OPENXR_LIB_PATH})

set(VENDOR_HEADERS_DIR "")
set(OPENXR_LOADER "openxr_loader")

## Setup the project sources
file(GLOB_RECURSE ANDROID_SOURCES ${CMAKE_CURRENT_LIST_DIR}/cpp/*.c**)
file(GLOB_RECURSE ANDROID_HEADERS ${CMAKE_CURRENT_LIST_DIR}/cpp/*.h**)

add_definitions(-DLYNX_VENDOR_ENABLED)
