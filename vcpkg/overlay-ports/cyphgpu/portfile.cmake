vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO Cyphall/CyphGPU
	REF "634cc19d695717e921a3aa4bea76ff0c2140ffab"
	SHA512 2f1c1461eab356e41ed3759311285f203ba914f01daf433ca5e93d702272f11c8d8cb4e72f09c8f1229390eb4b57f1889c7e465cb3012b9f42b3a7c250d506e9
	HEAD_REF master
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
	FEATURES
		tracy CYPHGPU_ENABLE_TRACY
		imgui-backend CYPHGPU_ENABLE_IMGUI_BACKEND
)

vcpkg_cmake_configure(
	SOURCE_PATH "${SOURCE_PATH}"
	OPTIONS
		-DCYPHGPU_INSTALL=ON
		${FEATURE_OPTIONS}
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup(CONFIG_PATH "lib/cmake/CyphGPU")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
