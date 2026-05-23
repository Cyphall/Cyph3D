cmake_minimum_required(VERSION 3.28)

project(imguizmo)

add_library(imguizmo)

target_sources(imguizmo PRIVATE
	"ImGuizmo.cpp"
)

target_sources(imguizmo PUBLIC FILE_SET HEADERS FILES
	"ImGuizmo.h"
)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(imguizmo PUBLIC imgui)

install(
	TARGETS imguizmo
	EXPORT imguizmoTargets
	FILE_SET HEADERS
)

install(
	EXPORT imguizmoTargets
	DESTINATION "lib/cmake/imguizmo"
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/imguizmoConfig.cmake"
	INSTALL_DESTINATION "lib/cmake/imguizmo"
)

install(
	FILES
	"${CMAKE_CURRENT_BINARY_DIR}/imguizmoConfig.cmake"
	DESTINATION "lib/cmake/imguizmo"
)