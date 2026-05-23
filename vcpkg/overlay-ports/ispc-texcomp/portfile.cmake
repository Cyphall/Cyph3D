vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO GameTechDev/ISPCTextureCompressor
	REF 691513b4fb406eccfc2f7d7f8213c8505ff5b897
	SHA512 ea875c90d256e66cc7f1186ca6132fde5abb959b9d523a34b55a4a594320940bcda89eff3b0da6197395c42be994503251acdacbd2a1590eac2b7129c275b454
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/config.cmake.in" DESTINATION "${SOURCE_PATH}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(
	SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup(CONFIG_PATH "lib/cmake/ispc-texcomp")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/license.txt")