cmake_minimum_required(VERSION 3.27)
project(systemProgLab2)

set(CMAKE_CXX_STANDARD 17)

add_executable(systemProgLab2FirstTask main.cpp
        check.hpp)
add_executable(systemProgLab2SecondTask main2.cpp
        check.hpp)
add_executable(systemProgLab2ThirdTask main3.cpp
        check.hpp)