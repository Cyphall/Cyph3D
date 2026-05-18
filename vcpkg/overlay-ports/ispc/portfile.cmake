set(VCPKG_BUILD_TYPE release)
set(VCPKG_POLICY_MISMATCHED_NUMBER_OF_BINARIES enabled)

if (VCPKG_TARGET_IS_WINDOWS)
	vcpkg_download_distfile(
		ARCHIVE
		URLS "https://github.com/ispc/ispc/releases/download/v${VERSION}/ispc-v${VERSION}-windows.zip"
		FILENAME "ispc-v${VERSION}-windows.zip"
		SHA512 e0bc4c2bff63317fd9a73727c9228f3287abe6261bf58017e9f98cc962b3ec53cdd0d7cc248fbe6a64e72911fcca7d30d10164f8545d3c0d4a91c1e1d0e6b107
	)
elseif (VCPKG_TARGET_IS_LINUX)
	vcpkg_download_distfile(
		ARCHIVE
		URLS "https://github.com/ispc/ispc/releases/download/v${VERSION}/ispc-v${VERSION}-linux.tar.gz"
		FILENAME "ispc-v${VERSION}-linux.tar.gz"
		SHA512 0e8efa2f3c195f2e77625bccf42df89c492aecd7e442f05f2d48f695c36dc8f4f25c10c337f2ce3e8453023a685c6e98f67faa343c6132b948aa82459a527158
	)
else ()
	message(FATAL_ERROR "Unsupported platform")
endif ()

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