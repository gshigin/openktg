cmake_minimum_required(VERSION 3.11)

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)

project(openktg
    VERSION 1.0.0 
    LANGUAGES CXX
)

add_library(${PROJECT_NAME} 
    src/gentexture.cpp
    src/pixel.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# install(TARGETS OpenKTG ARCHIVE DESTINATION lib)
# install(FILES ${openKTG_HEADERS} DESTINATION include/OpenKTG)

# set(openKTG_DOC_FILES LICENSE README.txt)
# install(FILES ${openKTG_DOC_FILES} DESTINATION share/doc/OpenKTG)

#
# demo executable
add_executable(demo demo.cpp)
target_link_libraries(demo openktg)
