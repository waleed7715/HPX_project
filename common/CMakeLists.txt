# cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/Users/walee/Downloads/vcpkg/scripts/buildsystems/vcpkg.cmake

cmake_minimum_required(VERSION 3.17)
project(hpx_test VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force Release mode if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# Enable optimizations and debug info
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2 /Ob2)
    else()
        add_compile_options(-O3 -march=native -flto)
        add_link_options(-flto)
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if(MSVC)
        add_compile_options(/O2 /Ob2 /Zi)  # Optimize + debug info
        add_link_options(/DEBUG /OPT:REF /OPT:ICF)  # Generate PDB, enable optimizations
    else()
        add_compile_options(-O3 -march=native -g)  # Optimize + debug symbols
    endif()
endif()

# set(CMAKE_CXX_FLAGS "-march=native")

add_executable(generate_data generate_data.cpp)
