include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/toolchains/msvc.cmake")

cmake_path(CONVERT "$ENV{VCPKG_ROOT}" TO_CMAKE_PATH_LIST VCPKG_ROOT)
include("${VCPKG_ROOT}/scripts/toolchains/windows.cmake")