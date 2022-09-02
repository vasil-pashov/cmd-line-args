cmake_minimum_required(VERSION 3.24.0)
project(cmd_line_args
	VERSION 0.1.0
	LANGUAGES CXX
)
cmake_policy(SET CMP0077 NEW)
include(FetchContent)

option(CMD_LINE_ARGS_WITH_TESTS "Builds tests for the project" OFF)
option(CMD_LINE_ARGS_WITH_INSTALL "Generate install target" OFF)

add_library(${PROJECT_NAME} STATIC
	cpp/cmd_line_args.cpp
	include/cmd_line_args.h
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

set(FETCH_CONTENT_LIBS)
if(${CMD_LINE_ARGS_WITH_TESTS})
	set(doctest_version 2.4.9)
	FetchContent_Declare(
		doctest
		GIT_REPOSITORY https://github.com/doctest/doctest.git
		GIT_TAG "v${doctest_version}"
	)
	set(DOCTEST_NO_INSTALL ON CACHE BOOL "")
	set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE BOOL "")
	list(APPEND FETCH_CONTENT_LIBS doctest)
endif()

if(${CMD_LINE_ARGS_WITH_INSTALL})
	include(CMakePackageConfigHelpers)
	if(UNIX)
		include(GNUInstallDirs)
		set(include_install_dir ${CMAKE_INSTALL_INCLUDEDIR})
		set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
		set(lib_install_dir ${CMAKE_INSTALL_LIBDIR})
	else()
		set(include_install_dir "include")
		set(config_install_dir "lib/cmake/${PROJECT_NAME}")
		set(lib_install_dir "lib")
	endif()

	set(targets_export_name "${PROJECT_NAME}Targets")
	install(
		TARGETS ${PROJECT_NAME}
		EXPORT ${targets_export_name}
		ARCHIVE DESTINATION ${lib_install_dir}
		INCLUDES DESTINATION ${include_install_dir}
	)

	install(
		EXPORT ${targets_export_name}
		FILE "${targets_export_name}.cmake"
		NAMESPACE CmdLineArgs::
		DESTINATION ${config_install_dir}
	)

	install(
		FILES
			include/cmd_line_args.h
			DESTINATION ${include_install_dir}
	)

	configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
		"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
		INSTALL_DESTINATION "${config_install_dir}"
		NO_SET_AND_CHECK_MACRO
		NO_CHECK_REQUIRED_COMPONENTS_MACRO
	)

	write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
		COMPATIBILITY SameMinorVersion
	)

	install(FILES
		${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
		${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
		DESTINATION ${config_install_dir}
	)
endif()

if(NOT "${FETCH_CONTENT_LIBS}" STREQUAL "")
	FetchContent_MakeAvailable(FETCH_CONTENT_LIBS)
endif()

if(${CMD_LINE_ARGS_WITH_TESTS})
	include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
	enable_testing()
	add_subdirectory(tests)
endif()

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${config_install_dir}>
)