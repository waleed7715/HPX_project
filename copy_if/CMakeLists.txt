cmake_minimum_required(VERSION 3.17)
project(hpx_test VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force Release mode if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# Enable optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2 /Ob2)
    else()
        add_compile_options(-O3 -march=native)
    endif()
endif()

# set(CMAKE_CXX_FLAGS "-march=native")

find_package(HPX REQUIRED)
add_executable(copy_if_hpx_future_when_all copy_if_hpx_future_when_all.cpp)
target_link_libraries(copy_if_hpx_future_when_all HPX::hpx HPX::wrap_main HPX::iostreams_component)

find_package(HPX REQUIRED)
add_executable(copy_if_std_par copy_if_std_par.cpp)

find_package(HPX REQUIRED)
add_executable(sys_info_test sys_info_test.cpp)
target_link_libraries(sys_info_test HPX::hpx HPX::wrap_main HPX::iostreams_component)
