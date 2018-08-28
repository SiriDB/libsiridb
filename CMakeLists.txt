cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
project(siridb VERSION 0.9.4)

if (NOT DEFINED CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

configure_file(
	"${CMAKE_MODULE_PATH}/version.h.in"
	"${CMAKE_SOURCE_DIR}/include/version.h"
)

find_package(qpack REQUIRED)

set(siridb_src_dir "${CMAKE_SOURCE_DIR}/src")
file(GLOB siridb_src ${siridb_src_dir}/*.c)

add_library(siridb STATIC ${siridb_src})
target_include_directories(siridb PRIVATE ${qpack_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include)
set_target_properties(siridb PROPERTIES SUFFIX ".so")
target_link_libraries(siridb PRIVATE qpack)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(
	TARGETS siridb
	EXPORT siridb-targets
	ARCHIVE DESTINATION lib
)

install(
	DIRECTORY ${CMAKE_SOURCE_DIR}/include/
	DESTINATION include/siridb
)

set(CONF_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/cmake/${CMAKE_PROJECT_NAME}")

write_basic_package_version_file(
	"${CMAKE_BINARY_DIR}/siridb-config-version.cmake"
	COMPATIBILITY AnyNewerVersion
)

export(EXPORT siridb-targets FILE ${CMAKE_BINARY_DIR}/siridb-targets.cmake)

configure_package_config_file(
	"${CMAKE_MODULE_PATH}/siridb-config.cmake.in"
	"${CMAKE_BINARY_DIR}/siridb-config.cmake"
	INSTALL_DESTINATION ${CONF_INSTALL_DIR}
	PATH_VARS CMAKE_INSTALL_FULL_INCLUDEDIR CMAKE_INSTALL_FULL_LIBDIR
)

install(EXPORT siridb-targets FILE siridb-targets.cmake DESTINATION ${CONF_INSTALL_DIR})

install(FILES ${CMAKE_BINARY_DIR}/siridb-config.cmake ${CMAKE_BINARY_DIR}/siridb-config-version.cmake DESTINATION ${CONF_INSTALL_DIR})
