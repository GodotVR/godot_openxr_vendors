cmake_minimum_required(VERSION 3.22.1)

## Android OpenXR loader library
find_package(OpenXR REQUIRED CONFIG)

set(VENDOR_HEADERS_DIR "")
set(OPENXR_LOADER "OpenXR::openxr_loader")

## Setup the project sources
file(GLOB_RECURSE ANDROID_SOURCES ${CMAKE_CURRENT_LIST_DIR}/cpp/*.c**)
file(GLOB_RECURSE ANDROID_HEADERS ${CMAKE_CURRENT_LIST_DIR}/cpp/*.h**)

add_definitions(-DANDROID_VENDOR_ENABLED)
