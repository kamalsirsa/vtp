# Add a library target called envdlg_xxx
set(ENVDLG_SOURCE_FILES
	BuildingDlg3d.cpp CameraDlg.cpp ContourDlg.cpp DistanceDlg3d.cpp EphemDlg.cpp FeatureTableDlg3d.cpp LayerDlg.cpp LinearStructDlg3d.cpp LocationDlg.cpp
	LODDlg.cpp OptionsDlg.cpp PlantDlg.cpp ScenarioParamsDialog.cpp ScenarioSelectDialog.cpp SceneGraphDlg.cpp StartupDlg.cpp StyleDlg.cpp
	TerrManDlg.cpp TextureDlg.cpp TimeDlg.cpp TParamsDlg.cpp UtilDlg.cpp VehicleDlg.cpp)

if(NVPERFSDK_FOUND AND NVIDIA_PERFORMANCE_MONITORING)
	set(ENVDLG_SOURCE_FILES ${ENVDLG_SOURCE_FILES} PerformanceMonitor.cpp)
endif(NVPERFSDK_FOUND AND NVIDIA_PERFORMANCE_MONITORING)

set(ENVDLG_HEADER_FILES
	BuildingDlg3d.h CameraDlg.h ContourDlg.h DistanceDlg3d.h EphemDlg.h FeatureTableDlg3d.h LayerDlg.h LinearStructDlg3d.h LocationDlg.h
	LODDlg.h OptionsDlg.h PlantDlg.h ScenarioParamsDialog.h ScenarioSelectDialog.h SceneGraphDlg.h StartupDlg.h StyleDlg.h
	TerrManDlg.h TParamsDlg.h TextureDlg.h TimeDlg.h UtilDlg.h VehicleDlg.h)

if(NVPERFSDK_FOUND AND NVIDIA_PERFORMANCE_MONITORING)
	set(ENVDLG_HEADER_FILES ${ENVDLG_HEADER_FILES} PerformanceMonitor.h)
endif(NVPERFSDK_FOUND AND NVIDIA_PERFORMANCE_MONITORING)

if(BUILD_ENVIRO_OSG)
	if(MSVC)
		add_library(envdlg ${ENVDLG_SOURCE_FILES} ${ENVDLG_HEADER_FILES} wx_headers.cpp)
		set_source_files_properties(${ENVDLG_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
		set_source_files_properties(wx_headers.cpp PROPERTIES COMPILE_FLAGS /Ycwx/wxprec.h)
	else(MSVC)
		add_library(envdlg ${ENVDLG_SOURCE_FILES} ${ENVDLG_HEADER_FILES})
	endif(MSVC)

	# Specify common preprocessor definitions for this target
	set_property(TARGET envdlg APPEND PROPERTY COMPILE_DEFINITIONS VTLIB_OSG=1)

	# Performance monitoring
	if(NVPERFSDK_FOUND AND NVIDIA_PERFORMANCE_MONITORING)
		set_property(TARGET envdlg APPEND PROPERTY COMPILE_DEFINITIONS NVIDIA_PERFORMANCE_MONITORING=1)
	endif(NVPERFSDK_FOUND AND NVIDIA_PERFORMANCE_MONITORING)

endif(BUILD_ENVIRO_OSG)

# Set up include directories for all targets at this level
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/icons)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/bitmap)

if(wxWidgets_FOUND)
	include(${wxWidgets_USE_FILE})
endif(wxWidgets_FOUND)

if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif(GDAL_FOUND)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

if(BUILD_ENVIRO_OSG) 
	if(OSG_FOUND)
		include_directories(${OSG_INCLUDE_DIR})
	endif(OSG_FOUND)
endif(BUILD_ENVIRO_OSG)

find_path(MINI_INCLUDE_DIR mini/mini.h DOC "Directory containing mini/mini.h")
if (MINI_INCLUDE_DIR)
	# Add the mini include directory to the compilers search path 
	include_directories(${MINI_INCLUDE_DIR})
endif (MINI_INCLUDE_DIR)

if(NVPERFSDK_FOUND AND NVIDIA_PERFORMANCE_MONITORING)
	include_directories(${NVPERFSDK_INCLUDE_DIR})
endif(NVPERFSDK_FOUND AND NVIDIA_PERFORMANCE_MONITORING)



