cmake_minimum_required(VERSION 2.6)
project(VTerrain)

option(BUILD_ENVIRO_OSG "Build the OSG variant of Enviro" ON)
option(BUILD_CMANAGER_OSG "Build the OSG variant of CManager" ON)

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
