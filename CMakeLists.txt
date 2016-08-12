
cmake_minimum_required(VERSION 3.2)

project(mempool)

set(WHICH_CPP C-11)

include_directories(${WHICH_CPP})

add_definitions(-std=c++11)

add_executable(benchmark
    benchmark.cpp
    ${WHICH_CPP}/MemoryPool.hpp ${WHICH_CPP}/MemoryPool.inl
    StackAlloc.hpp)

add_executable(mptest
    test.cpp
    ${WHICH_CPP}/MemoryPool.hpp ${WHICH_CPP}/MemoryPool.inl)
