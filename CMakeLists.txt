cmake_minimum_required(VERSION 3.13)

project(mondrian)

add_library(mondrianl src/mondrian.c src/utils.c)
add_executable(mondrian src/main.c src/mondrian.c src/utils.c)
