vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY ONLY_DYNAMIC_CRT)
set(VCPKG_BUILD_TYPE release)
set(VCPKG_POLICY_MISMATCHED_NUMBER_OF_BINARIES enabled)
set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)

if (VCPKG_TARGET_IS_WINDOWS)
	vcpkg_download_distfile(
		ARCHIVE
		URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-windows-x86_64.zip"
		FILENAME "slang-${VERSION}-windows-x86_64.zip"
		SHA512 8129340303cdb8769c73677e106a04ab969eeb46f3b0feb9d8f60503498af7207dab281d4d5e845d44272111935f84be6e174bfa62f82ed3e33cec3ce9315921
	)
elseif (VCPKG_TARGET_IS_OSX)
	vcpkg_download_distfile(
		ARCHIVE
		URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-macos-aarch64.zip"
		FILENAME "slang-${VERSION}-macos-aarch64.zip"
		SHA512 a626354b10449415093e6466000f1c8296e7cf83ac241a34791080a1bcfcc86afd64dc4616984d60d9ebf755769dac87f2d682e45135f152864733f250704dc0
	)
elseif (VCPKG_TARGET_IS_LINUX)
	vcpkg_download_distfile(
		ARCHIVE
		URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-linux-x86_64.zip"
		FILENAME "slang-${VERSION}-linux-x86_64.zip"
		SHA512 7153e5912cd381d24e4507dde69cd7582937cc9c5476dd05cd0caf487b38aeca6332a0c3592f6e4610cec002d46581e7aa1d64522b60a3d48a121344493cfaec
	)
endif ()

vcpkg_extract_source_archive(
	SOURCE_PATH
	ARCHIVE "${ARCHIVE}"
	NO_REMOVE_ONE_LEVEL
)

set(VCPKG_FIXUP_MACHO_RPATH OFF)

file(GLOB SLANGC "${SOURCE_PATH}/*/slangc*")
file(COPY ${SLANGC} DESTINATION "${CURRENT_PACKAGES_DIR}/tools/${PORT}")

file(GLOB SLANG_COMPILER "${SOURCE_PATH}/*/*slang-compiler*")
file(COPY ${SLANG_COMPILER} DESTINATION "${CURRENT_PACKAGES_DIR}/tools/${PORT}")

file(GLOB SLANG_GLSLANG "${SOURCE_PATH}/*/*slang-glslang*")
file(COPY ${SLANG_GLSLANG} DESTINATION "${CURRENT_PACKAGES_DIR}/tools/${PORT}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
