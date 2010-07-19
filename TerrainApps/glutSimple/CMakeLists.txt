find_package(GLUT)
find_package(OpenGL)

if(BUILD_GLUTSIMPLE_OSG AND GLUT_FOUND)
	add_executable(glutSimple_osg ${ENVIRO_SOURCE_FILES} app.cpp)

	# Internal library dependencies for this target
	target_link_libraries(glutSimple_osg minidata vtdata vtlib_osg xmlhelper)

	# Specify common preprocessor definitions for this target
	set_property(TARGET glutSimple_osg APPEND PROPERTY COMPILE_DEFINITIONS VTLIB_OSG=1 FREEGLUT_LIB_PRAGMAS=0)


	# Windows specific stuff
	if (WIN32)
		set_property(TARGET glutSimple_osg APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
		set_property(TARGET glutSimple_osg APPEND PROPERTY LINK_FLAGS_DEBUG /NODEFAULTLIB:msvcrt)
	endif (WIN32)

	# External libraries for this target
	target_link_libraries(glutSimple_osg ${GLUT_LIBRARIES})
	target_link_libraries(glutSimple_osg ${OPENGL_LIBRARIES})
	
	if(OSG_FOUND)
		target_link_libraries(glutSimple_osg ${OSG_ALL_LIBRARIES})
	endif (OSG_FOUND)
	
	if(BZIP2_FOUND)
		target_link_libraries(glutSimple_osg ${BZIP2_LIBRARIES})
	endif(BZIP2_FOUND)

	if(CURL_FOUND)
		target_link_libraries(glutSimple_osg ${CURL_LIBRARY})
	endif(CURL_FOUND)

	if(GDAL_FOUND)
		target_link_libraries(glutSimple_osg ${GDAL_LIBRARIES})
	endif (GDAL_FOUND)

	if(ZLIB_FOUND)
		target_link_libraries(glutSimple_osg ${ZLIB_LIBRARIES})
	endif(ZLIB_FOUND)

	if(JPEG_FOUND)
		target_link_libraries(glutSimple_osg ${JPEG_LIBRARY})
	endif(JPEG_FOUND)

	if(PNG_FOUND)
		target_link_libraries(glutSimple_osg ${PNG_LIBRARY})
	endif(PNG_FOUND)
	
	find_library(MINI_LIBRARY libMini DOC "Path to libMini")
	if(MINI_LIBRARY)
		target_link_libraries(glutSimple_osg ${MINI_LIBRARY})
	endif(MINI_LIBRARY)

	# Set up include directories for all targets at this level
	include_directories(${GLUT_INCLUDE_DIR})
	if(OSG_FOUND)
		include_directories(${OSG_INCLUDE_DIR})
	endif(OSG_FOUND)

	if(GDAL_FOUND)
		include_directories(${GDAL_INCLUDE_DIR})
	endif(GDAL_FOUND)

	if(ZLIB_FOUND)
		include_directories(${ZLIB_INCLUDE_DIR})
	endif(ZLIB_FOUND)

endif(BUILD_GLUTSIMPLE_OSG AND GLUT_FOUND)

