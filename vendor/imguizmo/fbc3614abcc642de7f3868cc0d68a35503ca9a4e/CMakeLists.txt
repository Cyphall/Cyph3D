cmake_minimum_required(VERSION 3.19)

project(imguizmo)

add_library(imguizmo STATIC src/ImGuizmo.cpp)

target_link_libraries(imguizmo PUBLIC imgui)

target_include_directories(imguizmo PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")