cmake_minimum_required(VERSION 3.4)
project(Poisson)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(Poisson main.cpp poisson.h)
