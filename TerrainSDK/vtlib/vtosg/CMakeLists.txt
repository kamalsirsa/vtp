set(VTLIB_OSG_SOURCE_FILES ImageOSG.cpp MeshMat.cpp NodeOSG.cpp SceneOSG.cpp SimpleInterimShadowTechnique.cpp StructureShadowsOSG.cpp)
set(VTLIB_OSG_HEADER_FILES ImageOSG.h MathOSG.h MeshMat.h NodeOSG.h SceneOSG.h SimpleInterimShadowTechnique.h StructureShadowsOSG.h)


# Add a library target called vtlib_osg

if(MSVC)
	add_library(vtlib_osg ${VTLIB_CORE_SOURCE_FILES} ${VTLIB_CORE_SOURCE_FILES_NON_PCH} ${VTLIB_CORE_HEADER_FILES}
			${VTLIB_OSG_SOURCE_FILES} ${VTLIB_OSG_HEADER_FILES} ../pre.cpp)
	set_source_files_properties(${VTLIB_CORE_SOURCE_FILES} ${VTLIB_OSG_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuvtlib/vtlib.h)
	set_source_files_properties(../pre.cpp PROPERTIES COMPILE_FLAGS /Ycvtlib/vtlib.h)
else(MSVC)
	add_library(vtlib_osg ${VTLIB_CORE_SOURCE_FILES} ${VTLIB_CORE_HEADER_FILES} ${VTLIB_OSG_SOURCE_FILES} ${VTLIB_OSG_HEADER_FILES})
endif(MSVC)

include_directories(${TERRAIN_SDK_ROOT} ${TERRAIN_SDK_ROOT}/vtlib)

if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif (GDAL_FOUND)

# Find the directory containing surfgrid.h
find_path(QUIKGRID_INCLUDE_PATH surfgrid.h DOC "Directory containing surfgrid.h")
if (QUIKGRID_INCLUDE_PATH)
	# Add the QuikGrid include directory to the compilers search path 
	include_directories(${QUIKGRID_INCLUDE_PATH})
endif (QUIKGRID_INCLUDE_PATH)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

if(OSG_FOUND)
	include_directories(${OSG_INCLUDE_DIR})
endif(OSG_FOUND)

# Find the directory containing mini/mini.h
find_path(MINI_INCLUDE_PATH mini/mini.h DOC "Directory containing mini/mini.h")
if (MINI_INCLUDE_PATH)
	# Add the mini include directory to the compilers search path 
	include_directories(${MINI_INCLUDE_PATH})
endif (MINI_INCLUDE_PATH)

# Windows specific stuff
if (WIN32)
	set_property(TARGET vtlib_osg APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
endif (WIN32)

# Specify normal preprocessor definition for this target
	set_property(TARGET vtlib_osg APPEND PROPERTY COMPILE_DEFINITIONS VTLIB_OSG=1)

# Specify debug preprocessor definitions for this target
set_property(TARGET vtlib_osg APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG VTDEBUG)

