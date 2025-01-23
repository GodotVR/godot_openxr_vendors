# Non functional cmake build file used to provide Android Studio editor support to the project.
cmake_minimum_required(VERSION 3.22.1)
project(godotopenxrvendors LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GODOT_CPP_DIR "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/godot-cpp")
set(OPENXR_HEADERS_DIR "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/openxr/include")
set(COMMON_LIB_HEADERS_DIR "${CMAKE_CURRENT_LIST_DIR}/src/main/cpp/include")

# Get sources
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/main/cpp/*.c**)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_LIST_DIR}/src/main/cpp/*.h**)

add_library(${PROJECT_NAME}
        SHARED
        ${SOURCES}
        ${HEADERS}
)

target_include_directories(${PROJECT_NAME}
        SYSTEM PUBLIC
        "${GODOT_CPP_DIR}/include"
        "${GODOT_CPP_DIR}/gen/include"
        "${GODOT_CPP_DIR}/gdextension"
        ${OPENXR_HEADERS_DIR}
        ${COMMON_LIB_HEADERS_DIR}
)

add_definitions(-DDEBUG_ENABLED)
