vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO CedricGuillemet/ImGuizmo
	REF fbc3614abcc642de7f3868cc0d68a35503ca9a4e
	SHA512 c9d57575478590d59d019fb35a6179ff73654c99c3a170849e8197b4be4c478a86aa118764fed96bdb659031a45de8572a7089f0dcff3554f3320d955e14cdf0
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