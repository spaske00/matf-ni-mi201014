cmake_minimum_required(VERSION 3.20)
project(marko_spasic_ni2020)
add_compile_options(-finput-charset=UTF-8)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_COMPILER g++-11)
set(CMAKE_CXX_LIB)
set(CMAKE_CXX_STANDARD 20)


if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -march=native")
    message('Building release')
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -fno-sanitize-recover=all")
    message('Building debug')
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fno-rtti -flto -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident ")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
include_directories(common)
add_executable(k_means_cpp_cpu_sequential src/k_means_cpp_cpu_sequential/main.cpp)
add_executable(k_means_cpp_cpu_parallel src/k_means_cpp_cpu_parallel/main.cpp)
add_executable(k_means_taskflow_cpu_parallel src/k_means_taskflow_cpu_parallel/main.cpp)

target_link_libraries(k_means_cpp_cpu_parallel pthread)
target_link_libraries(k_means_taskflow_cpu_parallel pthread)
# target_link_libraries(k_means_cpp_cpu_sequential)







