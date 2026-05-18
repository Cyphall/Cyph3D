include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/toolchains/clang-cl.cmake")

if (Z_VCPKG_ROOT_DIR)
	set(VCPKG_ROOT ${Z_VCPKG_ROOT_DIR})
elseif (_VCPKG_ROOT_DIR)
	set(VCPKG_ROOT ${_VCPKG_ROOT_DIR})
endif ()
include("${VCPKG_ROOT}/scripts/toolchains/windows.cmake")