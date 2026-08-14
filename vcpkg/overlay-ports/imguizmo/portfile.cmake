vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO CedricGuillemet/ImGuizmo
	REF "${VERSION}"
	SHA512 0f5ed80c74a67dc2711372307c9c90dd366f6c480a4a6e08f4a2275beab629d6399e2e1ad0c33e47a07d57737e9f3146aa4c1e0073964e949716c8a246ff5180
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