set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(VCPKG_C_FLAGS "/Zc:__cplusplus /utf-8")
set(VCPKG_CXX_FLAGS "/Zc:__cplusplus /utf-8")

if (Z_VCPKG_ROOT_DIR)
	set(VCPKG_ROOT ${Z_VCPKG_ROOT_DIR})
elseif (_VCPKG_ROOT_DIR)
	set(VCPKG_ROOT ${_VCPKG_ROOT_DIR})
endif ()
include("${VCPKG_ROOT}/scripts/toolchains/windows.cmake")