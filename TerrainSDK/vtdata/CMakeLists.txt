# Add a library target called vtdata
add_library(vtdata
		Building.cpp ByteOrder.cpp ChunkLOD.cpp ChunkUtil.cpp Content.cpp CubicSpline.cpp DataPath.cpp DLG.cpp
		DxfParser.cpp ElevationGrid.cpp ElevationGridBT.cpp ElevationGridDEM.cpp ElevationGridIO.cpp FeatureGeom.cpp
		Features.cpp Fence.cpp FilePath.cpp Geodesic.cpp GEOnet.cpp HeightField.cpp Icosa.cpp LevellerTag.cpp
		LocalConversion.cpp LULC.cpp MathTypes.cpp Matrix.cpp Plants.cpp PolyChecker.cpp Projections.cpp QuikGrid.cpp
		RoadMap.cpp SPA.cpp StructArray.cpp StructImport.cpp Structure.cpp Triangulate.cpp TripDub.cpp Unarchive.cpp
		UtilityMap.cpp Vocab.cpp vtDIB.cpp vtLog.cpp vtString.cpp vtTime.cpp vtTin.cpp vtUnzip.cpp WFSClient.cpp

		Array.h Building.h ByteOrder.h ChunkLOD.h ChunkUtil.h config_vtdata.h Content.h CubicSpline.h DataPath.h
		DLG.h DxfParser.h ElevationGrid.h Features.h Fence.h FilePath.h GEOnet.h HeightField.h Icosa.h LevellerTag.h
		LocalConversion.h LULC.h Mainpage.h MathTypes.h Plants.h PolyChecker.h Projections.h QuikGrid.h RoadMap.h
		Selectable.h SPA.h StatePlane.h StructArray.h Structure.h Triangulate.h TripDub.h Unarchive.h UtilityMap.h
		Vocab.h vtDIB.h vtLog.h vtString.h vtTime.h vtTin.h vtUnzip.h WFSClient.h

		triangle/triangle.c triangle/triangle.h)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

if(JPEG_FOUND)
	include_directories(${JPEG_INCLUDE_DIR})
endif(JPEG_FOUND)

if(PNG_FOUND)
	include_directories(${PNG_PNG_INCLUDE_DIR})
endif(PNG_FOUND)

if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif (GDAL_FOUND)

if(BZIP2_FOUND)
	include_directories(${BZIP2_INCLUDE_DIR})
endif(BZIP2_FOUND)

if(CURL_FOUND)
	include_directories(${CURL_INCLUDE_DIRS})
endif (CURL_FOUND)

# Find the directory containing curl/curl.h
find_path(NETCDF_INCLUDE_PATH netcdf.h.h DOC "Directory containing netcdf.h")
if (NETCDF_INCLUDE_PATH)
	# Add the netcdf include directory to the compilers search path 
	include_directories(${NETCDF_INCLUDE_PATH})
endif (NETCDF_INCLUDE_PATH)

# Find the directory containing surfgrid.h
find_path(QUIKGRID_INCLUDE_PATH surfgrid.h DOC "Directory containing surfgrid.h")
if (QUIKGRID_INCLUDE_PATH)
	# Add the QuikGrid include directory to the compilers search path 
	include_directories(${QUIKGRID_INCLUDE_PATH})
endif (QUIKGRID_INCLUDE_PATH)

include_directories(${TERRAIN_SDK_ROOT})

# Windows specific stuff
if (WIN32)
	set_property(TARGET vtdata APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
endif (WIN32)

# Specify debug preprocessor definitions for this target
set_property(TARGET vtdata APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG VTDEBUG)

