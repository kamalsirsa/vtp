cmake_minimum_required(VERSION 2.6)
project(VTerrain)

set(TERRAIN_SDK_ROOT "${VTerrain_SOURCE_DIR}/TerrainSDK")

# Find packages
find_package(wxWidgets COMPONENTS base core net xml adv html gl qa xrc aui richtext media odbc)
find_package(GDAL)
find_package(BZip2) # Beware this hides it's BZIP2_INCLUDE_DIR and BZIP2_LIBRARIES as advanced
find_package(CURL)
find_package(ZLIB)

add_subdirectory(TerrainSDK)
add_subdirectory(TerrainApps)
