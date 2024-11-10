find_package(CMakeRC CONFIG REQUIRED)

function(target_resources TARGET RESOURCE_NAME PREFIX)
	set(RESOURCE_FILE_PATHS_ABS)

	foreach (RESOURCE_FILE_PATH IN LISTS ARGN)
		cmake_path(ABSOLUTE_PATH RESOURCE_FILE_PATH NORMALIZE OUTPUT_VARIABLE RESOURCE_FILE_PATH_ABS)

		list(APPEND RESOURCE_FILE_PATHS_ABS "${RESOURCE_FILE_PATH_ABS}")
	endforeach ()

	cmrc_add_resource_library(${TARGET}_${RESOURCE_NAME}
		NAMESPACE ${RESOURCE_NAME}
		WHENCE "${CMAKE_CURRENT_SOURCE_DIR}/${PREFIX}"
		${RESOURCE_FILE_PATHS_ABS}
	)
	target_link_libraries(${TARGET} PRIVATE ${TARGET}_${RESOURCE_NAME})
endfunction()