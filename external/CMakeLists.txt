
add_subdirectory(SDL EXCLUDE_FROM_ALL)

set(SDLIMAGE_VENDORED ON)
set(SDLIMAGE_AVIF OFF)	# disable formats we don't use to make the build faster and smaller.
set(SDLIMAGE_BMP OFF)
set(SDLIMAGE_JPEG OFF)
set(SDLIMAGE_WEBP OFF)
add_subdirectory(SDL_image EXCLUDE_FROM_ALL)

set(IMGUI_SOURCES
    imgui/imgui_widgets.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_demo.cpp
    imgui/imgui.cpp
    imgui/imgui_tables.cpp
    imgui/backends/imgui_impl_sdl3.cpp
    imgui/backends/imgui_impl_sdlrenderer3.cpp
)

add_library(imgui STATIC EXCLUDE_FROM_ALL ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC imgui)
target_link_libraries(imgui PRIVATE SDL3::SDL3)