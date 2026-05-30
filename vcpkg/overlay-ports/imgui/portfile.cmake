vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO ocornut/imgui
	REF "v${VERSION}-docking"
	SHA512 927ecf72f00a228e0899d5b8008575b44748c49b083b9425b5f2a6b4490a9900eae111afad23f2bf0a1c9c62cf1fea80c903eb3076d7e7ea901a5625f09df78e
	HEAD_REF docking
	PATCHES include-path.patch imconfig.patch splitter-color.patch
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/config.cmake.in" DESTINATION "${SOURCE_PATH}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

file(RENAME "${SOURCE_PATH}/backends/imgui_impl_glfw.h" "${SOURCE_PATH}/imgui_impl_glfw.h")
file(RENAME "${SOURCE_PATH}/backends/imgui_impl_glfw.cpp" "${SOURCE_PATH}/imgui_impl_glfw.cpp")
file(RENAME "${SOURCE_PATH}/misc/cpp/imgui_stdlib.h" "${SOURCE_PATH}/imgui_stdlib.h")
file(RENAME "${SOURCE_PATH}/misc/cpp/imgui_stdlib.cpp" "${SOURCE_PATH}/imgui_stdlib.cpp")
file(RENAME "${SOURCE_PATH}/misc/freetype/imgui_freetype.h" "${SOURCE_PATH}/imgui_freetype.h")
file(RENAME "${SOURCE_PATH}/misc/freetype/imgui_freetype.cpp" "${SOURCE_PATH}/imgui_freetype.cpp")

vcpkg_cmake_configure(
	SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup(CONFIG_PATH "lib/cmake/imgui")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")