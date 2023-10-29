cmake_minimum_required(VERSION 3.24)

project(ISPCTextureCompressor)

add_library(ISPCTextureCompressor STATIC IMPORTED GLOBAL)

set_target_properties(ISPCTextureCompressor PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/ispc_texcomp.lib")

target_include_directories(ISPCTextureCompressor INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")