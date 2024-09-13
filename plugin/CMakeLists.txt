cmake_minimum_required(VERSION 3.22.1)

## Project definition
project(godotopenxrvendors LANGUAGES CXX)

## Common dependencies
include(${PROJECT_SOURCE_DIR}/src/main/common.cmake)

## Flavor dependencies
include(src/${FLAVOR}/${FLAVOR}.cmake)

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
        ${VENDOR_HEADERS_DIR}
        ${COMMON_LIB_HEADERS_DIR}
)

target_link_libraries(${PROJECT_NAME}
        android
        log
        ${GODOT-CPP}
        ${OPENXR_LOADER}
)

# Add the compile flags
set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS ${GODOT_COMPILE_FLAGS})
set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS ${GODOT_LINKER_FLAGS})
