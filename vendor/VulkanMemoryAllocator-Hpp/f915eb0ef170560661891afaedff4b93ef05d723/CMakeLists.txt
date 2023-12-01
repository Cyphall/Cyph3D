cmake_minimum_required(VERSION 3.19)

project(VulkanMemoryAllocator-Hpp)

add_library(VulkanMemoryAllocator-Hpp INTERFACE)

target_link_libraries(VulkanMemoryAllocator-Hpp INTERFACE VulkanMemoryAllocator)

target_include_directories(VulkanMemoryAllocator-Hpp INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")