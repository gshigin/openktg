cmake_minimum_required(VERSION 3.11)

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)

project(openktg
    VERSION 1.0.0 
    LANGUAGES CXX
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

add_library(${PROJECT_NAME} 
    src/gentexture.cpp
    src/pixel.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# demo executable
add_executable(demo demo.cpp)
target_link_libraries(demo openktg)

option(ENABLE_TESTS "Enable building tests" ON)

if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()

option(ENABLE_BENCHMARKS "Enable building benchmarks" ON)

if(ENABLE_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()