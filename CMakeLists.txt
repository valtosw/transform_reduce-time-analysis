cmake_minimum_required(VERSION 3.28)
project(LAB2)

set(CMAKE_CXX_STANDARD 20)

add_link_options(-static -static-libgcc -static-libstdc++)

add_executable(LAB2 main.cpp
        measuring_time.h)
