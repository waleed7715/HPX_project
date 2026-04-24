cmake_minimum_required(VERSION 3.20)
project(bench_tbb)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)

# vcpkg toolchain provides this
find_package(TBB REQUIRED)

add_executable(bench_tbb bench_tbb.cpp)

target_compile_definitions(bench_tbb PRIVATE
    COMMON_DATA_DIR="${CMAKE_SOURCE_DIR}/../common"  # adjust to wherever the .bin files are
)

target_link_libraries(bench_tbb PRIVATE
    TBB::tbb
)

target_compile_options(bench_tbb PRIVATE
    -O3
    -march=native
)

# Point to your shared helpers
target_include_directories(bench_tbb PRIVATE
    ${CMAKE_SOURCE_DIR}/../common   # adjust to wherever Random.hpp / Helper.hpp live
)
