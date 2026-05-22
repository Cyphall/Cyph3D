set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

if (Z_VCPKG_ROOT_DIR)
	set(VCPKG_ROOT ${Z_VCPKG_ROOT_DIR})
elseif (_VCPKG_ROOT_DIR)
	set(VCPKG_ROOT ${_VCPKG_ROOT_DIR})
endif ()
include("${VCPKG_ROOT}/scripts/toolchains/linux.cmake")