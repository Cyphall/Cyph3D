set(VCPKG_BUILD_TYPE release)

vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
	REF "v${VERSION}"
	SHA512 563acbcd8912d10d92c23715eba7acf0e7c1683af36021f415b36f359c2cce065f3906e395c32282a8410ec5c8179fbcb6412935c6629a49357475d4b4410e2a
	HEAD_REF master
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/config.cmake.in" DESTINATION "${SOURCE_PATH}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(
	SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup(PACKAGE_NAME "VulkanMemoryAllocator" CONFIG_PATH "lib/cmake/VulkanMemoryAllocator")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/lib")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
