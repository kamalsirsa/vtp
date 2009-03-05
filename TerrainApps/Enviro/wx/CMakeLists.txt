# Add a library target called envdlg_xxx
set(ENVDLG_SOURCE_FILES
	BuildingDlg3d.cpp CameraDlg.cpp DistanceDlg3d.cpp EphemDlg.cpp FeatureTableDlg3d.cpp LayerDlg.cpp LinearStructDlg3d.cpp LocationDlg.cpp
	LODDlg.cpp OptionsDlg.cpp PlantDlg.cpp ScenarioParamsDialog.cpp ScenarioSelectDialog.cpp SceneGraphDlg.cpp StartupDlg.cpp StyleDlg.cpp
	TerrManDlg.cpp TextureDlg.cpp TimeDlg.cpp TParamsDlg.cpp UtilDlg.cpp VehicleDlg.cpp)

set(ENVDLG_HEADER_FILES
	BuildingDlg3d.h CameraDlg.h DistanceDlg3d.h EphemDlg.h FeatureTableDlg3d.h LayerDlg.h LinearStructDlg3d.h LocationDlg.h LODDlg.h
	OptionsDlg.h PlantDlg.h ScenarioParamsDialog.h ScenarioSelectDialog.h SceneGraphDlg.h StartupDlg.h StyleDlg.h TerrManDlg.h TParamsDlg.h
	UtilDlg.h)

if(BUILD_ENVIRO_OSG)
	if(MSVC)
		add_library(envdlg ${ENVDLG_SOURCE_FILES} ${ENVDLG_HEADER_FILES} wx_headers.cpp)
		set_source_files_properties(${ENVDLG_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
		set_source_files_properties(wx_headers.cpp PROPERTIES COMPILE_FLAGS /Ycwx/wxprec.h)
	else(MSVC)
		add_library(envdlg ${ENVDLG_SOURCE_FILES} ${ENVDLG_HEADER_FILES})
	endif(MSVC)

	# Specify common preprocessor definitions for this target
	# set_property(TARGET envdlg APPEND PROPERTY COMPILE_DEFINITIONS)

	# Windows specific stuff
	if (WIN32)
		set_property(TARGET envdlg APPEND PROPERTY COMPILE_DEFINITIONS _UNICODE UNICODE WXUSINGDLL=1 VTLIB_OSG=1)
	endif (WIN32)

	# Specify debug preprocessor definitions for this target
	set_property(TARGET envdlg APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG VTDEBUG)
endif(BUILD_ENVIRO_OSG)

# Set up include directories for all targets at this level
include_directories(${TERRAIN_SDK_ROOT})

if(wxWidgets_FOUND)
	include_directories(${wxWidgets_INCLUDE_DIRS})
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

find_path(MINI_INCLUDE_PATH mini/mini.h DOC "Directory containing mini/mini.h")
if (MINI_INCLUDE_PATH)
	# Add the mini include directory to the compilers search path 
	include_directories(${MINI_INCLUDE_PATH})
endif (MINI_INCLUDE_PATH)


