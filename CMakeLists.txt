cmake_minimum_required(VERSION 3.13)

project(mondrian)

add_executable(mondrian src/mondrian.c)
add_executable(cat_mondrian src/cat_mondrian.c src/utils.c)
