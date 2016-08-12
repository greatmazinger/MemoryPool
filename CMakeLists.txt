
cmake_minimum_required(VERSION 3.2)

project(mempool)

include_directories(C-11)

add_definitions(-std=c++11)

add_executable(mempool_test test.cpp)
