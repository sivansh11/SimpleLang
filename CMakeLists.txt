cmake_minimum_required(VERSION 3.10)

project(simpleLang)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(simpleLang ${SRC_FILES})