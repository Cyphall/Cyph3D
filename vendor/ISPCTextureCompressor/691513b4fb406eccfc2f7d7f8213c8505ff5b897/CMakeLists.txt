cmake_minimum_required(VERSION 3.19)

set(CMAKE_ISPC_COMPILER_TARGET )

project(ISPCTextureCompressor LANGUAGES CXX ISPC)

add_library(ISPCTextureCompressor STATIC src/ispc_texcomp.cpp src/kernel.ispc)

target_include_directories(ISPCTextureCompressor PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")