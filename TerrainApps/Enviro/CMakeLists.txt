add_subdirectory(wx)

set(ENVIRO_SOURCE_FILES
	CarEngine.cpp EarthView.cpp Engines.cpp Enviro.cpp Hawaii.cpp Nevada.cpp Options.cpp SpecificTerrain.cpp
	Vehicles.cpp)
	
set(ENVIRO_SOURCE_FILES_WX
	wx/canvas.cpp wx/enviro_wdr.cpp wx/EnviroApp.cpp wx/EnviroFrame.cpp wx/EnviroGUI.cpp wx/StatusBar.cpp)
	
set(ENVIRO_HEADER_FILES
	CarEngine.h Engines.h Enviro.h enviroenum.h Hawaii.h Nevada.h Options.h PlantingOptions.h SpecificTerrain.h Vehicles.h)
	
set(ENVIRO_HEADER_FILES_WX
	wx/canvas.h wx/enviro_wdr.h wx/EnviroApp.h wx/EnviroFrame.h wx/EnviroGUI.h wx/menu_id.h wx/StatusBar.h wx/TextureDlg.h
	wx/TimeDlg.h wx/VehicleDlg.h)
	
set(ENVIRO_RC_FILES
	wx/enviro-wx.rc)
	
if(BUILD_ENVIRO_OSG)
	if(MSVC)
		add_executable(enviro_osg WIN32 ${ENVIRO_SOURCE_FILES} ${ENVIRO_SOURCE_FILES_WX} ${ENVIRO_HEADER_FILES}
			${ENVIRO_HEADER_FILES_WX} ${ENVIRO_RC_FILES} vtlib_headers.cpp wx/wx_headers.cpp)
		# When I can find a way of having telling CMake to pch files in the build directories (Debug Release etc.)
		# then I can have more than pch file. One possible way round thnis would be to force vtlib_headers.cpp
		# to be recompiled on every build
		#set_source_files_properties(${ENVIRO_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "/Yuvtlib/vtlib.h /Fpvtlib.pch")
		#set_source_files_properties(vtlib_headers.cpp PROPERTIES COMPILE_FLAGS "/Ycvtlib/vtlib.h /Fpvtlib.pch")
		set_source_files_properties(${ENVIRO_SOURCE_FILES_WX} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
		set_source_files_properties(wx/wx_headers.cpp PROPERTIES COMPILE_FLAGS /Ycwx/wxprec.h)
	else(MSVC)
		add_executable(enviro_osg WIN32 ${ENVIRO_SOURCE_FILES} ${ENVIRO_SOURCE_FILES_WX} ${ENVIRO_HEADER_FILES}
			${ENVIRO_HEADER_FILES_WX} ${ENVIRO_RC_FILES})
	endif(MSVC)

	# Internal dependencies for this target
	add_dependencies(enviro_osg envdlg minidata vtdata vtlib_osg vtui xmlhelper)

	# Specify common preprocessor definitions for this target
	set_property(TARGET enviro_osg APPEND PROPERTY COMPILE_DEFINITIONS VTLIB_OSG=1)

	# Windows specific stuff
	if (WIN32)
		set_property(TARGET enviro_osg APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE _UNICODE UNICODE WXUSINGDLL=1 wxUSE_NO_MANIFEST=1)
		set_property(TARGET enviro_osg APPEND PROPERTY LINK_FLAGS_DEBUG /NODEFAULTLIB:msvcrt;wxpngd;wxtiffd;wxjpegd;wxzlibd)
		set_property(TARGET enviro_osg APPEND PROPERTY LINK_FLAGS /NODEFAULTLIB:wxpng;wxtiff;wxjpeg;wxzlib)
	endif (WIN32)

	# Specify debug preprocessor definitions for this target
	set_property(TARGET enviro_osg APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG VTDEBUG)

	# External libraries for this target
	if(wxWidgets_FOUND)
		target_link_libraries(enviro_osg ${wxWidgets_LIBRARIES})
	endif(wxWidgets_FOUND)

	if(GDAL_FOUND)
		target_link_libraries(enviro_osg ${GDAL_LIBRARIES})
	endif (GDAL_FOUND)

	if(OSG_FOUND)
		target_link_libraries(enviro_osg ${OSG_ALL_LIBRARIES})
	endif (OSG_FOUND)
	
	if(BZIP2_FOUND)
		target_link_libraries(enviro_osg ${BZIP2_LIBRARIES})
	endif(BZIP2_FOUND)

	if(CURL_FOUND)
		target_link_libraries(enviro_osg ${CURL_LIBRARIES})
	endif(CURL_FOUND)

	if(ZLIB_FOUND)
		target_link_libraries(enviro_osg ${ZLIB_LIBRARIES})
	endif(ZLIB_FOUND)

	if(JPEG_FOUND)
		target_link_libraries(enviro_osg ${JPEG_LIBRARY})
	endif(JPEG_FOUND)

	if(PNG_FOUND)
		target_link_libraries(enviro_osg ${PNG_LIBRARY})
	endif(PNG_FOUND)
	
	if(QUIKGRID_FOUND)
		target_link_libraries(enviro_osg ${QUIKGRID_LIBRARIES})
	endif(QUIKGRID_FOUND)

	find_library(MINI_LIBRARY libMini DOC "Path to libMini")
	if(MINI_LIBRARY)
		target_link_libraries(enviro_osg ${MINI_LIBRARY})
	endif(MINI_LIBRARY)

endif(BUILD_ENVIRO_OSG)

# Set up include directories for all targets at this level
include_directories(${TERRAIN_SDK_ROOT})

if(wxWidgets_FOUND)
	include_directories(${wxWidgets_INCLUDE_DIRS})
endif(wxWidgets_FOUND)

if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif(GDAL_FOUND)

if(BUILD_ENVIRO_OSG) 
	if(OSG_FOUND)
		include_directories(${OSG_INCLUDE_DIR})
	endif(OSG_FOUND)
endif(BUILD_ENVIRO_OSG)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

find_path(GLEXT_INCLUDE_PATH GL/glext.h DOC "Directory containing GL/glext.h")
if (GLEXT_INCLUDE_PATH)
	include_directories(${GLEXT_INCLUDE_PATH})
endif (GLEXT_INCLUDE_PATH)
