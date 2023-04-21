cmake_minimum_required(VERSION 3.24)

project(bc7enc_rdo)

add_library(bc7enc_rdo STATIC src/bc7enc.cpp src/rgbcx.cpp)

target_include_directories(bc7enc_rdo PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")