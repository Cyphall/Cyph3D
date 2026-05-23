cmake_minimum_required(VERSION 3.28)

project(ispc-texcomp LANGUAGES CXX ISPC)

add_library(ispc-texcomp)

target_sources(ispc-texcomp PRIVATE
	"ispc_texcomp/kernel.ispc"
	"ispc_texcomp/kernel_astc.ispc"
	"ispc_texcomp/ispc_texcomp.cpp"
	"ispc_texcomp/ispc_texcomp_astc.cpp"
)

target_compile_options(ispc-texcomp PRIVATE $<$<COMPILE_LANGUAGE:ISPC>:--opt=fast-math>)

target_sources(ispc-texcomp PUBLIC FILE_SET HEADERS BASE_DIRS "ispc_texcomp" FILES
	"ispc_texcomp/ispc_texcomp.h"
)

install(
	TARGETS ispc-texcomp
	EXPORT ispc-texcompTargets
	FILE_SET HEADERS
)

install(
	EXPORT ispc-texcompTargets
	DESTINATION "lib/cmake/ispc-texcomp"
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/ispc-texcompConfig.cmake"
	INSTALL_DESTINATION "lib/cmake/ispc-texcomp"
)

install(
	FILES
	"${CMAKE_CURRENT_BINARY_DIR}/ispc-texcompConfig.cmake"
	DESTINATION "lib/cmake/ispc-texcomp"
)