
set(VTLIB_OSG_SOURCE_FILES ImageOSG.cpp Material.cpp Mesh.cpp GeometryUtils.cpp NodeOSG.cpp NodeLog.cpp SceneOSG.cpp
		SimpleInterimShadowTechnique.cpp StructureShadowsOSG.cpp LightSpacePerspectiveShadowTechnique.cpp
		ExternalHeightField3d.cpp OSGEventHandler.cpp ScreenCaptureHandler.cpp SaveImageOSG.cpp)
set(VTLIB_OSG_HEADER_FILES ImageOSG.h MathOSG.h Material.h Mesh.h GeometryUtils.h NodeOSG.h SceneOSG.h
		SimpleInterimShadowTechnique.h StructureShadowsOSG.h LightSpacePerspectiveShadowTechnique.h
		ExternalHeightField3d.h OSGEventHandler.h ScreenCaptureHandler.h SaveImageOSG.h)


# Add a library target called vtlib

if(MSVC)
	add_library(vtlib ${VTLIB_CORE_SOURCE_FILES} ${VTLIB_CORE_SOURCE_FILES_NON_PCH} ${VTLIB_CORE_HEADER_FILES}
			${VTLIB_OSG_SOURCE_FILES} ${VTLIB_OSG_HEADER_FILES} ../pre.cpp)
	set_source_files_properties(${VTLIB_CORE_SOURCE_FILES} ${VTLIB_OSG_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuvtlib/vtlib.h)
	set_source_files_properties(../pre.cpp PROPERTIES COMPILE_FLAGS /Ycvtlib/vtlib.h)
else(MSVC)
	add_library(vtlib ${VTLIB_CORE_SOURCE_FILES} ${VTLIB_CORE_SOURCE_FILES_NON_PCH} ${VTLIB_CORE_HEADER_FILES}
	${VTLIB_OSG_SOURCE_FILES} ${VTLIB_OSG_HEADER_FILES})
endif(MSVC)


if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif (GDAL_FOUND)

if(QUIKGRID_FOUND)
	include_directories(${QUIKGRID_INCLUDE_DIR})
	set_property(TARGET vtlib APPEND PROPERTY COMPILE_DEFINITIONS SUPPORT_QUIKGRID)
endif(QUIKGRID_FOUND)

if(CURL_FOUND)
	include_directories(${CURL_INCLUDE_DIR})
	set_property(TARGET vtlib APPEND PROPERTY COMPILE_DEFINITIONS SUPPORT_CURL)
	if(CURL_IS_STATIC)
		set_property(TARGET vtlib APPEND PROPERTY COMPILE_DEFINITIONS CURL_STATICLIB)
	endif(CURL_IS_STATIC)
endif (CURL_FOUND)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

if(OSG_FOUND)
	include_directories(${OSG_INCLUDE_DIR})
endif(OSG_FOUND)

if(OSGEARTH_FOUND)
	include_directories(${OSGEARTH_INCLUDE_DIR})
endif(OSGEARTH_FOUND)

if(MINI_FOUND)
	include_directories(${MINI_INCLUDE_DIR})
endif(MINI_FOUND)

# Find the directory containing GL/glext.h
find_path(GLEXT_INCLUDE_DIR GL/glext.h DOC "Directory containing GL/glext.h")
if (GLEXT_INCLUDE_DIR)
	# Add the glext include directory to the compilers search path
	include_directories(${GLEXT_INCLUDE_DIR})
endif (GLEXT_INCLUDE_DIR)

# Windows specific stuff
if (WIN32)
	set_property(TARGET vtlib APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
endif (WIN32)

# Specify normal preprocessor definition for this target
if(OSGEARTH_FOUND)
	set_property(TARGET vtlib APPEND PROPERTY COMPILE_DEFINITIONS USE_OSGEARTH=1)
endif(OSGEARTH_FOUND)
