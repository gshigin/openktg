find_package(Catch2 3 CONFIG QUIET)

if (NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)
endif()

add_executable(tests test_pixel.cpp test_utils.cpp)
target_link_libraries(tests Catch2::Catch2WithMain openktg)