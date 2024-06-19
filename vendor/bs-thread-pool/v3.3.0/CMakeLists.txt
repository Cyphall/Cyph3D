cmake_minimum_required(VERSION 3.28)

project(bs-thread-pool)

add_library(bs-thread-pool INTERFACE)

target_include_directories(bs-thread-pool INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")