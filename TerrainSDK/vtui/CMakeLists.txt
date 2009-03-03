# Add a library target called vtui
set(VTUI_SOURCE_FILES 
		AutoDialog.cpp BuildingDlg.cpp ColorMapDlg.cpp DistanceDlg.cpp DoubleProgDlg.cpp FeatureTableDlg.cpp Helper.cpp
		InstanceDlg.cpp Joystick.cpp LinearStructDlg.cpp ProfileDlg.cpp ProfileEditDlg.cpp ProjectionDlg.cpp
		SizeDlg.cpp StatePlaneDlg.cpp TagDlg.cpp vtui_wdr.cpp)
set(VTUI_HEADER_FILES 
		AutoDialog.h BuildingDlg.h ColorMapDlg.h DistanceDlg.h DoubleProgDlg.h FeatureTableDlg.h Helper.h InstanceDlg.h
		Joystick.h LinearStructDlg.h ProfileDlg.h ProfileEditDlg.h ProjectionDlg.h SizeDlg.h StatePlaneDlg.h
		TagDlg.h vtui_wdr.h)
if(MSVC)
	add_library(vtui ${VTUI_SOURCE_FILES} ${VTUI_HEADER_FILES} pre.cpp)
	set_source_files_properties(${VTUI_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
	set_source_files_properties(pre.cpp PROPERTIES COMPILE_FLAGS /Ycwx/wxprec.h)
else(MSVC)
	add_library(vtui ${VTUI_SOURCE_FILES} ${VTUI_HEADER_FILES})
endif(MSVC)

include_directories(${TERRAIN_SDK_ROOT})

# Find the directory containing wx/setup.h
find_path(WX_SETUP_PATH wx/setup.h DOC "Directory containing wx/setup.h")
if (WX_SETUP_PATH)
	# Add the wx setup include directory to the compilers search path 
	include_directories(${WX_SETUP_PATH})
endif (WX_SETUP_PATH)

# Find the directory containing wx/wx.h
find_path(WX_INCLUDE_PATH wx/wx.h DOC "Directory containing wx/wx.h")
if (WX_INCLUDE_PATH)
	# Add the wx setup include directory to the compilers search path 
	include_directories(${WX_INCLUDE_PATH})
endif (WX_INCLUDE_PATH)

# Find the directory containing gdal.h
find_path(GDAL_INCLUDE_PATH gdal.h DOC "Directory containing gdal.h")
if (GDAL_INCLUDE_PATH)
	# Add the gdal include directory to the compilers search path 
	include_directories(${GDAL_INCLUDE_PATH})
endif (GDAL_INCLUDE_PATH)

# Find the directory containing zlib.h
find_path(ZLIB_INCLUDE_PATH zlib.h DOC "Directory containing zlib.h")
# N.B. You cannot use ${xxxx} in an if (or other expression) to test if xxxx has been set
# if CMake cannot find the variable named xxxx it will use the direct value ${xxxx} which will never be a boolean true
# Enclose the include_directories command in a conditional to stop cmake complaing when it has not been set
if (ZLIB_INCLUDE_PATH)
	# Add the zlib include directory to the compilers search path 
	include_directories(${ZLIB_INCLUDE_PATH})
endif (ZLIB_INCLUDE_PATH)

# Find the directory containing surfgrid.h
find_path(QUIKGRID_INCLUDE_PATH surfgrid.h DOC "Directory containing surfgrid.h")
if (QUIKGRID_INCLUDE_PATH)
	# Add the QuikGrid include directory to the compilers search path 
	include_directories(${QUIKGRID_INCLUDE_PATH})
endif (QUIKGRID_INCLUDE_PATH)

# Windows specific stuff
if (WIN32)
	set_property(TARGET vtui APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE _UNICODE UNICODE WXUSINGDLL=1)
endif (WIN32)

# Specify debug preprocessor definitions for this target
set_property(TARGET vtui APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG VTDEBUG)

