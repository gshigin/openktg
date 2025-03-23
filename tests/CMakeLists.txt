find_package(GTest CONFIG QUIET)

if (NOT GTest_FOUND)
    message(STATUS "GTest not found: Fetching GTest")

    include(FetchContent)
    FetchContent_Declare(
        GTest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.16.0
    )
    FetchContent_MakeAvailable(GTest)
else()
    message(STATUS "GTest found")
endif()

add_executable(tests test_pixel.cpp test_utils.cpp test_end_to_end.cpp)
target_link_libraries(tests GTest::gtest_main openktg)

enable_testing()
add_test(NAME RunTests COMMAND tests)