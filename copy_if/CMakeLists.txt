# cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DHPX_DIR="/home/waleed/hpx_master/lib/cmake/HPX" -DCMAKE_TOOLCHAIN_FILE=/home/waleed/vcpkg/scripts/buildsystems/vcpkg.cmake

cmake_minimum_required(VERSION 3.17)
project(hpx_test VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to shared headers
set(COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)

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
        add_compile_options(/O2 /Ob2 /Zi)
        add_link_options(/DEBUG /OPT:REF /OPT:ICF)
    else()
        add_compile_options(-O3 -march=native -g)
    endif()
endif()

# ---------------- VTune (optional) ----------------
find_path(ITT_INCLUDE_DIR ittnotify.h
    PATHS
    $ENV{VTUNE_PROFILER_DIR}/include
    "C:/Program Files (x86)/Intel/oneAPI/vtune/2025.10/include"
    "C:/Program Files/Intel/oneAPI/vtune/2025.10/include"
    /opt/intel/oneapi/vtune/2025.10/include
)

find_library(ITT_LIBRARY
    NAMES libittnotify ittnotify
    PATHS
    $ENV{VTUNE_PROFILER_DIR}/lib64
    "C:/Program Files (x86)/Intel/oneAPI/vtune/2025.10/lib64"
    "C:/Program Files/Intel/oneAPI/vtune/2025.10/lib64"
    /opt/intel/oneapi/vtune/2025.10/lib64
)

if(ITT_INCLUDE_DIR AND ITT_LIBRARY)
    set(HAVE_VTUNE TRUE)
    message(STATUS "Found Intel ITT API: ${ITT_LIBRARY}")
else()
    set(HAVE_VTUNE FALSE)
    message(STATUS "Intel ITT API not found - VTune instrumentation disabled")
endif()

# ---------------- std ----------------
# std::execution::par uses TBB via PSTL when using GCC compiler 
find_package(TBB REQUIRED)
add_executable(copy_if_std_par copy_if_std_par.cpp)
target_include_directories(copy_if_std_par PRIVATE ${COMMON_INCLUDE_DIR})
target_compile_definitions(copy_if_std_par
    PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
)
if(HAVE_VTUNE)
    target_include_directories(copy_if_std_par PRIVATE ${ITT_INCLUDE_DIR})
    target_link_libraries(copy_if_std_par PRIVATE ${ITT_LIBRARY})
    target_compile_definitions(copy_if_std_par PRIVATE HAVE_VTUNE=1)
endif()

target_link_libraries(copy_if_std_par PRIVATE
    TBB::tbb
)

add_executable(copy_if_std_seq copy_if_std_seq.cpp)
target_include_directories(copy_if_std_seq PRIVATE ${COMMON_INCLUDE_DIR})
target_compile_definitions(copy_if_std_seq
    PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
)
if(HAVE_VTUNE)
    target_include_directories(copy_if_std_seq PRIVATE ${ITT_INCLUDE_DIR})
    target_link_libraries(copy_if_std_seq PRIVATE ${ITT_LIBRARY})
    target_compile_definitions(copy_if_std_seq PRIVATE HAVE_VTUNE=1)
endif()

# ---------------- OpenMP ----------------
find_package(OpenMP REQUIRED)
add_executable(copy_if_openmp copy_if_openmp.cpp)
target_include_directories(copy_if_openmp PRIVATE ${COMMON_INCLUDE_DIR})
target_compile_definitions(copy_if_openmp
    PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
)
target_link_libraries(copy_if_openmp PRIVATE OpenMP::OpenMP_CXX)
if(HAVE_VTUNE)
    target_include_directories(copy_if_openmp PRIVATE ${ITT_INCLUDE_DIR})
    target_link_libraries(copy_if_openmp PRIVATE ${ITT_LIBRARY})
    target_compile_definitions(copy_if_openmp PRIVATE HAVE_VTUNE=1)
endif()

# ---------------- TBB ----------------
find_package(TBB REQUIRED)
add_executable(copy_if_tbb copy_if_tbb.cpp)
target_include_directories(copy_if_tbb PRIVATE ${COMMON_INCLUDE_DIR})
target_compile_definitions(copy_if_tbb
    PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
)
target_link_libraries(copy_if_tbb PRIVATE TBB::tbb)
if(HAVE_VTUNE)
    target_include_directories(copy_if_tbb PRIVATE ${ITT_INCLUDE_DIR})
    target_link_libraries(copy_if_tbb PRIVATE ${ITT_LIBRARY})
    target_compile_definitions(copy_if_tbb PRIVATE HAVE_VTUNE=1)
endif()

target_link_libraries(copy_if_tbb PRIVATE
    TBB::tbb
)

# ---------------- Taskflow ----------------
find_package(Taskflow REQUIRED)
add_executable(copy_if_taskflow copy_if_taskflow.cpp)
target_include_directories(copy_if_taskflow PRIVATE ${COMMON_INCLUDE_DIR})
target_compile_definitions(copy_if_taskflow
    PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
)
target_link_libraries(copy_if_taskflow PRIVATE Taskflow::Taskflow)
if(HAVE_VTUNE)
    target_include_directories(copy_if_taskflow PRIVATE ${ITT_INCLUDE_DIR})
    target_link_libraries(copy_if_taskflow PRIVATE ${ITT_LIBRARY})
    target_compile_definitions(copy_if_taskflow PRIVATE HAVE_VTUNE=1)
endif()

# ---------------- HPX ----------------
find_package(HPX REQUIRED)

add_executable(copy_if_hpx copy_if_hpx.cpp)
target_include_directories(copy_if_hpx PRIVATE ${COMMON_INCLUDE_DIR})
target_compile_definitions(copy_if_hpx
    PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
)
target_link_libraries(copy_if_hpx PRIVATE HPX::hpx HPX::wrap_main HPX::iostreams_component)
if(HAVE_VTUNE)
    target_include_directories(copy_if_hpx PRIVATE ${ITT_INCLUDE_DIR})
    target_link_libraries(copy_if_hpx PRIVATE ${ITT_LIBRARY})
    target_compile_definitions(copy_if_hpx PRIVATE HAVE_VTUNE=1)
endif()
