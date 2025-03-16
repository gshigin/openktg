find_package(benchmark CONFIG QUIET)

if (NOT benchmark_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.1
    )
    FetchContent_MakeAvailable(benchmark)
endif()

file(GLOB BENCHMARK_SOURCES "*.cpp")

foreach(BENCHMARK_FILE ${BENCHMARK_SOURCES})
    get_filename_component(BENCHMARK_NAME ${BENCHMARK_FILE} NAME_WE)

    add_executable(${BENCHMARK_NAME} ${BENCHMARK_FILE})

    target_link_libraries(${BENCHMARK_NAME} PRIVATE benchmark::benchmark openktg)
endforeach()