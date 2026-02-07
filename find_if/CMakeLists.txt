# cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/Users/walee/Downloads/vcpkg/scripts/buildsystems/vcpkg.cmake

cmake_minimum_required(VERSION 3.17)
project(hpx_test VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to shared headers
set(COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)

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
        add_compile_options(/O2 /Ob2 /Zi)
        add_link_options(/DEBUG /OPT:REF /OPT:ICF)
    else()
        add_compile_options(-O3 -march=native -g)
    endif()
endif()

# ---------------- std ----------------
# add_executable(find_if_std_par find_if_std_par.cpp)
# target_include_directories(find_if_std_par PRIVATE ${COMMON_INCLUDE_DIR})
# target_compile_definitions(find_if_std_par
#     PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
# )

# add_executable(find_if_std_seq find_if_std_seq.cpp)
# target_include_directories(find_if_std_seq PRIVATE ${COMMON_INCLUDE_DIR})
# target_compile_definitions(find_if_std_seq
#     PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
# )

# ---------------- OpenMP ----------------
# find_package(OpenMP REQUIRED)
# add_executable(find_if_openmp find_if_openmp.cpp)
# target_include_directories(find_if_openmp PRIVATE ${COMMON_INCLUDE_DIR})
# target_compile_definitions(find_if_openmp
#     PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
# )
# target_link_libraries(find_if_openmp OpenMP::OpenMP_CXX)

# ---------------- TBB ----------------
# find_package(TBB REQUIRED)
# add_executable(find_if_tbb find_if_tbb.cpp)
# target_include_directories(find_if_tbb PRIVATE ${COMMON_INCLUDE_DIR})
# target_compile_definitions(find_if_tbb
#     PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
# )
# target_link_libraries(find_if_tbb PRIVATE TBB::tbb)

# ---------------- Taskflow ----------------
# find_package(Taskflow REQUIRED)
# add_executable(find_if_taskflow find_if_taskflow.cpp)
# target_include_directories(find_if_taskflow PRIVATE ${COMMON_INCLUDE_DIR})
# target_compile_definitions(find_if_taskflow
#     PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
# )
# target_link_libraries(find_if_taskflow PRIVATE Taskflow::Taskflow)

# ---------------- VTune (optional) ----------------
# find_path(ITT_INCLUDE_DIR ittnotify.h
#     PATHS
#     $ENV{VTUNE_PROFILER_DIR}/include
#     "C:/Program Files (x86)/Intel/oneAPI/vtune/latest/include"
#     "C:/Program Files/Intel/oneAPI/vtune/latest/include"
#     /opt/intel/oneapi/vtune/latest/include
# )

# find_library(ITT_LIBRARY
#     NAMES libittnotify ittnotify
#     PATHS
#     $ENV{VTUNE_PROFILER_DIR}/lib64
#     "C:/Program Files (x86)/Intel/oneAPI/vtune/latest/lib64"
#     "C:/Program Files/Intel/oneAPI/vtune/latest/lib64"
#     /opt/intel/oneapi/vtune/latest/lib64
# )

# if(ITT_INCLUDE_DIR AND ITT_LIBRARY)
#     set(HAVE_VTUNE TRUE)
#     message(STATUS "Found Intel ITT API: ${ITT_LIBRARY}")
# else()
#     set(HAVE_VTUNE FALSE)
# endif()

# ---------------- HPX ----------------
find_package(HPX REQUIRED)

add_executable(find_if_hpx find_if_hpx.cpp)
target_include_directories(find_if_hpx PRIVATE ${COMMON_INCLUDE_DIR})
target_compile_definitions(find_if_hpx
    PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
)
target_link_libraries(find_if_hpx HPX::hpx HPX::wrap_main HPX::iostreams_component)
if(HAVE_VTUNE)
    target_include_directories(find_if_hpx PRIVATE ${ITT_INCLUDE_DIR})
    target_link_libraries(find_if_hpx ${ITT_LIBRARY})
    target_compile_definitions(find_if_hpx PRIVATE HAVE_VTUNE=1)
endif()

# add_executable(find_if_test find_if_test.cpp)
# target_include_directories(find_if_test PRIVATE ${COMMON_INCLUDE_DIR})
# target_compile_definitions(find_if_test
#     PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
# )
# target_link_libraries(find_if_test HPX::hpx HPX::wrap_main HPX::iostreams_component)
# if(HAVE_VTUNE)
#     target_include_directories(find_if_test PRIVATE ${ITT_INCLUDE_DIR})
#     target_link_libraries(find_if_test ${ITT_LIBRARY})
#     target_compile_definitions(find_if_test PRIVATE HAVE_VTUNE=1)
# endif()

# add_executable(find_if_hpx_clean find_if_hpx_clean.cpp)
# target_include_directories(find_if_hpx_clean PRIVATE ${COMMON_INCLUDE_DIR})
# target_compile_definitions(find_if_hpx_clean
#     PRIVATE COMMON_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common"
# )
# target_link_libraries(find_if_hpx_clean HPX::hpx HPX::wrap_main HPX::iostreams_component)
# if(HAVE_VTUNE)
#     target_include_directories(find_if_hpx_clean PRIVATE ${ITT_INCLUDE_DIR})
#     target_link_libraries(find_if_hpx_clean ${ITT_LIBRARY})
#     target_compile_definitions(find_if_hpx_clean PRIVATE HAVE_VTUNE=1)
# endif()
