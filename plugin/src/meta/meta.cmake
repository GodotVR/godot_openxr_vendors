cmake_minimum_required(VERSION 3.22.1)

## OpenXR Mobile loader library
# Sets the path to the OpenXR mobile library directory.
set(OPENXR_MOBILE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../thirdparty/ovr_openxr_mobile_sdk/OpenXR")
set(OPENXR_MOBILE_HEADERS_DIR "${OPENXR_MOBILE_ROOT_DIR}/Include" CACHE STRING "")

set(OPENXR_MOBILE_LIB_PATH "${OPENXR_MOBILE_ROOT_DIR}/Libs/Android/${ANDROID_ABI}/${OPENXR_MOBILE_LIB_BUILD_TYPE}/libopenxr_loader.so")
add_library(openxr_loader
        SHARED
        IMPORTED GLOBAL
)
set_target_properties(openxr_loader PROPERTIES IMPORTED_LOCATION ${OPENXR_MOBILE_LIB_PATH})

set(VENDOR_HEADERS_DIR "${OPENXR_MOBILE_HEADERS_DIR}")
set(OPENXR_LOADER "openxr_loader")

## Setup the project sources
file(GLOB_RECURSE ANDROID_SOURCES ${CMAKE_CURRENT_LIST_DIR}/cpp/*.c**)
file(GLOB_RECURSE ANDROID_HEADERS ${CMAKE_CURRENT_LIST_DIR}/cpp/*.h**)

add_definitions(-DMETA_VENDOR_ENABLED)
