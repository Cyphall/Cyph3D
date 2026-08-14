vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO ocornut/imgui
	REF "v${VERSION}-docking"
	SHA512 7eddcdb475f1db1fc8242d918533b955c964d2267abe713bdf23f8e2444770946d3c79c7855e360bab6168e36231b95bd05a84106c08f876dcd53daac9caccac
	HEAD_REF docking
	PATCHES imconfig.patch splitter-color.patch
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/config.cmake.in" DESTINATION "${SOURCE_PATH}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(
	SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup(CONFIG_PATH "lib/cmake/imgui")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")