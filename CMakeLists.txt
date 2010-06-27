cmake_minimum_required(VERSION 2.6)
project(VTerrain)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
endif(NOT CMAKE_BUILD_TYPE)

option(BUILD_OSG_VERSIONS "Build OSG versions of the applications" ON)

if(BUILD_OSG_VERSIONS)
	set(BUILD_ENVIRO_OSG ON)
	set(BUILD_CMANAGER_OSG ON)
	set(BUILD_GLUTSIMPLE_OSG ON)
	set(BUILD_WXSIMPLE_OSG ON)
endif(BUILD_OSG_VERSIONS)

set(TERRAIN_SDK_ROOT "${CMAKE_SOURCE_DIR}/TerrainSDK")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

# Top level preprocessor definitions
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG VTDEBUG)
if(UNIX)
	set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS VTUNIX=1)
endif(UNIX)
if(APPLE)
	set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS __DARWIN_OSX__=1)
endif(APPLE)

# Find packages - most of these modules hide their include and library properties behind the advanced flag
find_package(wxWidgets COMPONENTS base core net xml adv html gl qa xrc aui richtext media)
find_package(GDAL)
find_package(BZip2)
find_package(CURL)
find_package(ZLIB)
find_package(JPEG)
find_package(PNG)
find_package(OSG)
find_package(QuikGrid)

add_subdirectory(TerrainSDK)
add_subdirectory(TerrainApps)
