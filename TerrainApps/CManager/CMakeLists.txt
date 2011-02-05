set(CMANAGER_SOURCE_FILES
	app.cpp canvas.cpp cmanager_wdr.cpp frame.cpp ItemGroup.cpp LightDlg.cpp ModelDlg.cpp PropDlg.cpp SceneGraphDlg.cpp
	TagDlg.cpp TreeView.cpp)
	
set(CMANAGER_HEADER_FILES
	app.h canvas.h cmanager_wdr.h frame.h ItemGroup.h LightDlg.h menu_id.h ModelDlg.h PropDlg.h SceneGraphDlg.h TagDlg.h TreeView.h)
	
set(CMANAGER_RC_FILES
	CManager.rc)
	
if(BUILD_CMANAGER_OSG)
	if(MSVC)
		add_executable(cmanager_osg WIN32 ${CMANAGER_SOURCE_FILES} ${CMANAGER_HEADER_FILES} ${CMANAGER_RC_FILES} pre.cpp)
		set_source_files_properties(${CMANAGER_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
		set_source_files_properties(pre.cpp PROPERTIES COMPILE_FLAGS /Ycwx/wxprec.h)
	else(MSVC)
		add_executable(cmanager_osg WIN32 ${CMANAGER_SOURCE_FILES} ${CMANAGER_HEADER_FILES}
			${CMANAGER_RC_FILES})
	endif(MSVC)

	# Internal library dependencies for this target
	target_link_libraries(cmanager_osg vtlib_osg vtdata vtui xmlhelper minidata)

	# Specify common preprocessor definitions for this target
	set_property(TARGET cmanager_osg APPEND PROPERTY COMPILE_DEFINITIONS VTLIB_OSG=1)

	# Windows specific stuff
	if (WIN32)
		set_property(TARGET cmanager_osg APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
		set_property(TARGET cmanager_osg APPEND PROPERTY LINK_FLAGS_DEBUG /NODEFAULTLIB:msvcrt;wxpngd;wxtiffd;wxjpegd;wxzlibd)
		set_property(TARGET cmanager_osg APPEND PROPERTY LINK_FLAGS /NODEFAULTLIB:wxpng;wxtiff;wxjpeg;wxzlib)
	endif (WIN32)


	# External libraries for this target
	if(wxWidgets_FOUND)
		include(${wxWidgets_USE_FILE})
		target_link_libraries(cmanager_osg ${wxWidgets_LIBRARIES})
	endif(wxWidgets_FOUND)

	if(GDAL_FOUND)
		target_link_libraries(cmanager_osg ${GDAL_LIBRARIES})
	endif (GDAL_FOUND)

	if(OSG_FOUND)
		target_link_libraries(cmanager_osg ${OSG_ALL_LIBRARIES})
	endif (OSG_FOUND)
	
	if (OSGEARTH_FOUND)
		target_link_libraries(cmanager_osg ${OSGEARTH_ALL_LIBRARIES})
	endif(OSGEARTH_FOUND)
	
	if(BZIP2_FOUND)
		target_link_libraries(cmanager_osg ${BZIP2_LIBRARIES})
	endif(BZIP2_FOUND)

	if(ZLIB_FOUND)
		target_link_libraries(cmanager_osg ${ZLIB_LIBRARIES})
	endif(ZLIB_FOUND)

	if(JPEG_FOUND)
		target_link_libraries(cmanager_osg ${JPEG_LIBRARY})
	endif(JPEG_FOUND)

	if(PNG_FOUND)
		target_link_libraries(cmanager_osg ${PNG_LIBRARY})
	endif(PNG_FOUND)
	
	find_library(MINI_LIBRARY libMini DOC "Path to libMini")
	if(MINI_LIBRARY)
		target_link_libraries(cmanager_osg ${MINI_LIBRARY})
	endif(MINI_LIBRARY)

	if(CURL_FOUND)
		target_link_libraries(cmanager_osg ${CURL_LIBRARY})
	endif(CURL_FOUND)

endif(BUILD_CMANAGER_OSG)

# Set up include directories for all targets at this level
if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif(GDAL_FOUND)

if(BUILD_CMANAGER_OSG) 
	if(OSG_FOUND)
		include_directories(${OSG_INCLUDE_DIR})
	endif(OSG_FOUND)
endif(BUILD_CMANAGER_OSG)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)



