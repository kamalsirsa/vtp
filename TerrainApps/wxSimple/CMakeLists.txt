set(WXSIMPLE_SOURCE_FILES app.cpp canvas.cpp frame.cpp) 
set(WXSIMPLE_HEADER_FILES app.h canvas.h frame.h)

if(BUILD_WXSIMPLE_OSG)
	if(MSVC)
		add_executable(wxSimple_osg WIN32 ${WXSIMPLE_SOURCE_FILES} ${WX_SIMPLE_HEADER_FILES} wx_headers.cpp)
		#set_source_files_properties(${WXSIMPLE_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "/Yuwx/wxprec.h")
		#set_source_files_properties(wx_headers.cpp PROPERTIES COMPILE_FLAGS "/Ycwx/wxprec.h")
	else(MSVC)
		add_executable(wxSimple_osg WIN32 ${WXSIMPLE_SOURCE_FILES} ${WX_SIMPLE_HEADER_FILES})
	endif(MSVC)

	# Internal dependencies for this target
	add_dependencies(wxSimple_osg minidata vtdata vtlib_osg xmlhelper)

	# Specify common preprocessor definitions for this target
	set_property(TARGET wxSimple_osg APPEND PROPERTY COMPILE_DEFINITIONS VTLIB_OSG=1)

	# Specify debug preprocessor definitions for this target
	set_property(TARGET wxSimple_osg APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG VTDEBUG)

	# Windows specific stuff
	if (WIN32)
		set_property(TARGET wxSimple_osg APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE _UNICODE UNICODE WXUSINGDLL=1)
		set_property(TARGET wxSimple_osg APPEND PROPERTY LINK_FLAGS_DEBUG /NODEFAULTLIB:msvcrt;wxpngd;wxtiffd;wxjpegd;wxzlibd)
		set_property(TARGET wxSimple_osg APPEND PROPERTY LINK_FLAGS /NODEFAULTLIB:wxpng;wxtiff;wxjpeg;wxzlib)
	endif (WIN32)

	# External libraries for this target
	if(wxWidgets_FOUND)
		target_link_libraries(wxSimple_osg ${wxWidgets_LIBRARIES})
	endif(wxWidgets_FOUND)

	if(GDAL_FOUND)
		target_link_libraries(wxSimple_osg ${GDAL_LIBRARY})
	endif (GDAL_FOUND)

	if(OSG_FOUND)
		target_link_libraries(wxSimple_osg ${OSG_ALL_LIBRARIES})
	endif (OSG_FOUND)
	
	if(BZIP2_FOUND)
		target_link_libraries(wxSimple_osg ${BZIP2_LIBRARIES})
	endif(BZIP2_FOUND)

	if(CURL_FOUND)
		target_link_libraries(wxSimple_osg ${CURL_LIBRARIES})
	endif(CURL_FOUND)

	if(ZLIB_FOUND)
		target_link_libraries(wxSimple_osg ${ZLIB_LIBRARIES})
	endif(ZLIB_FOUND)

	if(JPEG_FOUND)
		target_link_libraries(wxSimple_osg ${JPEG_LIBRARY})
	endif(JPEG_FOUND)

	if(PNG_FOUND)
		target_link_libraries(wxSimple_osg ${PNG_LIBRARY})
	endif(PNG_FOUND)
	
	find_library(QUIKGRID_LIBRARY QuikGrid DOC "Path to QuikGrid library")
	if(QUIKGRID_LIBRARY)
		target_link_libraries(wxSimple_osg ${QUIKGRID_LIBRARY})
	endif(QUIKGRID_LIBRARY)

	find_library(MINI_LIBRARY libMini DOC "Path to libMini")
	if(MINI_LIBRARY)
		target_link_libraries(wxSimple_osg ${MINI_LIBRARY})
	endif(MINI_LIBRARY)

endif(BUILD_WXSIMPLE_OSG)

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
