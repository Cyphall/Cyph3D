cmake_minimum_required(VERSION 3.19)

project(sigslot)

add_library(sigslot INTERFACE)

target_include_directories(sigslot INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")