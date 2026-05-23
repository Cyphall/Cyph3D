cmake_minimum_required(VERSION 3.28)

project(imgui)

add_library(imgui)

target_sources(imgui PRIVATE
	"imgui.cpp"
	"imgui_demo.cpp"
	"imgui_draw.cpp"
	"imgui_tables.cpp"
	"imgui_widgets.cpp"
	"imgui_impl_glfw.cpp"
	"imgui_stdlib.cpp"
	"imgui_freetype.cpp"
)

target_sources(imgui PUBLIC FILE_SET HEADERS FILES
	"imconfig.h"
	"imgui.h"
	"imgui_internal.h"
	"imstb_rectpack.h"
	"imstb_textedit.h"
	"imstb_truetype.h"
	"imgui_impl_glfw.h"
	"imgui_stdlib.h"
	"imgui_freetype.h"
)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(imgui PUBLIC glfw)
target_compile_definitions(imgui PUBLIC GLFW_INCLUDE_NONE)

find_package(Freetype REQUIRED)
target_link_libraries(imgui PRIVATE Freetype::Freetype)

find_package(glm CONFIG REQUIRED)
target_link_libraries(imgui PUBLIC glm::glm)

install(
	TARGETS imgui
	EXPORT imguiTargets
	FILE_SET HEADERS
)

install(
	EXPORT imguiTargets
	DESTINATION "lib/cmake/imgui"
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/imguiConfig.cmake"
	INSTALL_DESTINATION "lib/cmake/imgui"
)

install(
	FILES
	"${CMAKE_CURRENT_BINARY_DIR}/imguiConfig.cmake"
	DESTINATION "lib/cmake/imgui"
)