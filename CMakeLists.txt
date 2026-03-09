cmake_minimum_required(VERSION 3.27)
project(openktg)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

add_subdirectory(external)

project(openktg
    VERSION 0.0.0 
    LANGUAGES CXX
)

add_library(openktg
    src/gentexture.cpp
)

target_include_directories(openktg PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# demo executable
add_executable(demo demo.cpp)
target_link_libraries(demo openktg)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE SDL3_image::SDL3_image SDL3::SDL3 imgui openktg)