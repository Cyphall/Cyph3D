set(VCPKG_BUILD_TYPE release)
set(VCPKG_POLICY_MISMATCHED_NUMBER_OF_BINARIES enabled)

vcpkg_download_distfile(
	ARCHIVE
	URLS "https://github.com/ispc/ispc/releases/download/v${VERSION}/ispc-v${VERSION}-windows.zip"
	FILENAME "ispc-v${VERSION}-windows.zip"
	SHA512 e0bc4c2bff63317fd9a73727c9228f3287abe6261bf58017e9f98cc962b3ec53cdd0d7cc248fbe6a64e72911fcca7d30d10164f8545d3c0d4a91c1e1d0e6b107
)

vcpkg_extract_source_archive(
	SOURCE_PATH
	ARCHIVE "${ARCHIVE}"
)

file(COPY "${SOURCE_PATH}/bin" DESTINATION "${CURRENT_PACKAGES_DIR}")
file(COPY "${SOURCE_PATH}/include" DESTINATION "${CURRENT_PACKAGES_DIR}")
file(COPY "${SOURCE_PATH}/lib" DESTINATION "${CURRENT_PACKAGES_DIR}")

vcpkg_cmake_config_fixup(CONFIG_PATH "lib/cmake/ispc")

vcpkg_copy_tools(
	TOOL_NAMES ispc
	AUTO_CLEAN
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")