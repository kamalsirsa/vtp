cmake_minimum_required(VERSION 2.8)
project(VTP)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
endif(NOT CMAKE_BUILD_TYPE)

option(USE_OSG_STATS "Enable the OSG stats display" OFF)
if(USE_OSG_STATS)
	set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS USE_OSG_STATS)
endif(USE_OSG_STATS)

option(AVOID_OSG_INDICES "Don't use deprecated indices" OFF)
if(AVOID_OSG_INDICES)
	set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS AVOID_OSG_INDICES)
endif(AVOID_OSG_INDICES)

option(VISUAL_IMPACT_CALCULATOR "Enable experimental visual impact calculator" OFF)
if(VISUAL_IMPACT_CALCULATOR)
	set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS VISUAL_IMPACT_CALCULATOR)
endif(VISUAL_IMPACT_CALCULATOR)

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
set(wxWidgets_EXCLUDE_COMMON_LIBRARIES TRUE)
find_package(wxWidgets COMPONENTS base core net gl aui adv html)
find_package(GDAL)
find_package(BZip2)
find_package(CURL)
find_package(ZLIB)
find_package(JPEG)
find_package(PNG)
find_package(OSG)
find_package(QuikGrid)
find_package(osgEarth)
find_package(MINI)
find_package(OpenGL)

# Optionally use NVidia performance monitoring if present
find_path(NVPERFSDK_INCLUDE_DIR NVPerfSDK.h PATHS "c:/Program Files/NVIDIA Corporation/NVIDIA PerfSDK/inc")
if(NVPERFSDK_INCLUDE_DIR)
	find_library(NVPERFSDK_LIBRARY NVPerfSDK.lib PATHS "c:/Program Files/NVIDIA Corporation/NVIDIA PerfSDK/lib/winxp_x86")
	if(NVPERFSDK_LIBRARY)
		set(NVPERFSDK_FOUND 1)
		option(NVIDIA_PERFORMANCE_MONITORING "Enable NVIDIA peformance monitoring dialog")
	endif(NVPERFSDK_LIBRARY)
endif(NVPERFSDK_INCLUDE_DIR)

add_subdirectory(TerrainSDK)
add_subdirectory(TerrainApps)

option(USE_CPACK "Enable the CPack packager" OFF)
if(USE_CPACK)
	include(CPack)
endif(USE_CPACK)

