cmake_minimum_required(VERSION 3.24)

project(glad)

add_library(glad STATIC src/glad.c)

target_include_directories(glad PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")