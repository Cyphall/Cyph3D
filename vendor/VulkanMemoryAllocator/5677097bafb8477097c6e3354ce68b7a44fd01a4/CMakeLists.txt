cmake_minimum_required(VERSION 3.19)

project(VulkanMemoryAllocator)

add_library(VulkanMemoryAllocator INTERFACE)

target_include_directories(VulkanMemoryAllocator INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")