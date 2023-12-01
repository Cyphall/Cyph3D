cmake_minimum_required(VERSION 3.19)

project(imgui)

add_library(imgui STATIC src/imgui.cpp src/imgui_demo.cpp src/imgui_draw.cpp src/imgui_impl_glfw.cpp src/imgui_stdlib.cpp src/imgui_tables.cpp src/imgui_widgets.cpp src/imgui_freetype.cpp)

target_link_libraries(imgui PUBLIC glm::glm)

target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")