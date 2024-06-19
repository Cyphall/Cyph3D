cmake_minimum_required(VERSION 3.28)

# This separation is needed because otherwise the C++ module dependency scan for ispc_texcomp.cpp
# will happen before the ISPC file is compiled and its header file generated, causing the build to fail

# Internal ISPC implementation

project(ISPCTextureCompressorInternal LANGUAGES ISPC)

add_library(ISPCTextureCompressorInternal STATIC src/kernel.ispc)

# C++ interface

project(ISPCTextureCompressor)

add_library(ISPCTextureCompressor STATIC src/ispc_texcomp.cpp)

target_include_directories(ISPCTextureCompressor PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

target_link_libraries(ISPCTextureCompressor PRIVATE ISPCTextureCompressorInternal)