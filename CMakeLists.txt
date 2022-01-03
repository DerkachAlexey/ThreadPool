cmake_minimum_required(VERSION 3.10)
project(ThreadPool)

set(CMAKE_CXX_STANDARD 17)

add_library(ThreadPool STATIC ThreadPool.hpp ThreadPool.cpp)