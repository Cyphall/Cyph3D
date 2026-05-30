vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO CedricGuillemet/ImGuizmo
	REF be8aa4aeab86b402701c8c1df011bd8cd776760b
	SHA512 8fa89693e78a1fae5de8cbefc5e7ae102bac242f05ceabb43bb24d4e626175e97d1438acebe8aed965e21b616f28c7192776d3667e7b112af170ccb4da498a3c
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/config.cmake.in" DESTINATION "${SOURCE_PATH}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(
	SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup(CONFIG_PATH "lib/cmake/imguizmo")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")