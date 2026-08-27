cmake_minimum_required(VERSION 3.28)

project(VulkanMemoryAllocator)

add_library(VulkanMemoryAllocator INTERFACE)

target_sources(VulkanMemoryAllocator PUBLIC FILE_SET HEADERS BASE_DIRS "include" FILES
	"include/vk_mem_alloc.h"
)

install(
	TARGETS VulkanMemoryAllocator
	EXPORT VulkanMemoryAllocatorTargets
	FILE_SET HEADERS
)

install(
	EXPORT VulkanMemoryAllocatorTargets
	DESTINATION "lib/cmake/VulkanMemoryAllocator"
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/VulkanMemoryAllocatorConfig.cmake"
	INSTALL_DESTINATION "lib/cmake/VulkanMemoryAllocator"
)

install(
	FILES
	"${CMAKE_CURRENT_BINARY_DIR}/VulkanMemoryAllocatorConfig.cmake"
	DESTINATION "lib/cmake/VulkanMemoryAllocator"
)