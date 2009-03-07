cmake_minimum_required(VERSION 2.6)
project(VTerrain)

option(BUILD_OSG_VERSIONS "Build OSG versions of the applications" ON)

if(BUILD_OSG_VERSIONS)
	set(BUILD_ENVIRO_OSG ON)
	set(BUILD_CMANAGER_OSG ON)
	set(BUILD_GLUTSIMPLE_OSG ON)
	set(BUILD_WXSIMPLE_OSG ON)
endif(BUILD_OSG_VERSIONS)

set(TERRAIN_SDK_ROOT "${VTerrain_SOURCE_DIR}/TerrainSDK")
set(CMAKE_MODULE_PATH "${VTerrain_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

# Find packages - most of these modules hide their include and library properties behind the advanced flag
find_package(wxWidgets COMPONENTS base core net xml adv html gl qa xrc aui richtext media odbc)
find_package(GDAL)
find_package(BZip2)
find_package(CURL)
find_package(ZLIB)
find_package(JPEG)
find_package(PNG)
find_package(OSG)

add_subdirectory(TerrainSDK)
add_subdirectory(TerrainApps)
