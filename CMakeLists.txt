cmake_minimum_required(VERSION 3.11)

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")

add_library(${PROJECT_NAME} 
    src/legacy/gentexture.cpp
    src/core/pixel.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

option(ENABLE_TRACY "Enable Tracy Profiler" OFF) 

if(ENABLE_TRACY)
  message(STATUS "Tracy Profiler enabled")
    find_package(TracyClient CONFIG QUIET)

    if (NOT TracyClient_FOUND)
      message(STATUS "Tracy not found: Fetching Tracy")
      include(FetchContent)
      FetchContent_Declare(
          tracy
          GIT_REPOSITORY https://github.com/wolfpld/tracy.git
          GIT_TAG v0.11.1
      )
      FetchContent_MakeAvailable(tracy)
    else()  
        message(STATUS "Tracy found") 
    endif()
    
    add_definitions(-DTRACY_ENABLE)

    target_link_libraries(openktg TracyClient)
endif()

option(ENABLE_TESTS "Enable tests" ON)

if(ENABLE_TESTS)
    message(STATUS "Tests enabled")
    add_subdirectory(tests)
endif()

option(ENABLE_BENCHMARKS "Enable benchmarks" ON)

if(ENABLE_BENCHMARKS)
message(STATUS "Benchmarks enabled")
  add_subdirectory(benchmarks)
endif()

# demo executable
add_executable(demo demo.cpp)
target_link_libraries(demo openktg)