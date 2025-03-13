cmake_minimum_required(VERSION 3.25)
project(B)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=native")

add_executable(B corpus.cpp
        corpus.h
        main.cpp
)
