# Add a library target called vtdata
add_library(vtdata
		Building.cpp ByteOrder.cpp ChunkLOD.cpp ChunkUtil.cpp Content.cpp CubicSpline.cpp DataPath.cpp DLG.cpp
		DxfParser.cpp ElevationGrid.cpp ElevationGridBT.cpp ElevationGridDEM.cpp ElevationGridIO.cpp FeatureGeom.cpp
		Features.cpp Fence.cpp FilePath.cpp Geodesic.cpp GEOnet.cpp HeightField.cpp Icosa.cpp LevellerTag.cpp
		LocalConversion.cpp LULC.cpp MathTypes.cpp Matrix.cpp Plants.cpp PolyChecker.cpp Projections.cpp QuikGrid.cpp
		SPA.cpp StructArray.cpp StructImport.cpp Structure.cpp Triangulate.cpp TripDub.cpp Unarchive.cpp
		UtilityMap.cpp Vocab.cpp vtDIB.cpp vtLog.cpp vtString.cpp vtTime.cpp vtTin.cpp vtUnzip.cpp WFSClient.cpp

		Array.h Building.h ByteOrder.h ChunkLOD.h ChunkUtil.h config_vtdata.h Content.h CubicSpline.h DataPath.h
		DLG.h DxfParser.h ElevationGrid.h Features.h Fence.h FilePath.h GEOnet.h HeightField.h Icosa.h LevellerTag.h
		LocalConversion.h LULC.h Mainpage.h MathTypes.h Plants.h PolyChecker.h Projections.h QuikGrid.h RoadMap.h
		Selectable.h SPA.h StatePlane.h StructArray.h Structure.h Triangulate.h TripDub.h Unarchive.h UtilityMap.h
		Vocab.h vtDIB.h vtLog.h vtString.h vtTime.h vtTin.h vtUnzip.h WFSClient.h

		triangle/triangle.c triangle/triangle.h)

# Find the directory containing zlib.h
find_path(ZLIB_INCLUDE_PATH zlib.h DOC "Directory containing zlib.h")
# N.B. You cannot use ${xxxx} in an if (or other expression) to test if xxxx has been set
# if CMake cannot find the variable named xxxx it will use the direct value ${xxxx} which will never be a boolean true
# Enclose the include_directories command in a conditional to stop cmake complaing when it has not been set
if (ZLIB_INCLUDE_PATH)
	# Add the zlib include directory to the compilers search path 
	include_directories(${ZLIB_INCLUDE_PATH})
endif (ZLIB_INCLUDE_PATH)

# Find the directory containing jpeglib.h
find_path(JPEG_INCLUDE_PATH jpeglib.h DOC "Directory containing jpeglib.h")
if (JPEG_INCLUDE_PATH)
	# Add the jpeg include directory to the compilers search path 
	include_directories(${JPEG_INCLUDE_PATH})
endif (JPEG_INCLUDE_PATH)

# Find the directory containing png.h
find_path(PNG_INCLUDE_PATH png.h DOC "Directory containing png.h")
if (PNG_INCLUDE_PATH)
	# Add the png include directory to the compilers search path 
	include_directories(${PNG_INCLUDE_PATH})
endif (PNG_INCLUDE_PATH)

# Find the directory containing gdal.h
find_path(GDAL_INCLUDE_PATH gdal.h DOC "Directory containing gdal.h")
if (GDAL_INCLUDE_PATH)
	# Add the gdal include directory to the compilers search path 
	include_directories(${GDAL_INCLUDE_PATH})
endif (GDAL_INCLUDE_PATH)

# Find the directory containing bzlib.h
find_path(BZIP_INCLUDE_PATH bzlib.h DOC "Directory containing bzlib.h")
if (BZIP_INCLUDE_PATH)
	# Add the bzip include directory to the compilers search path 
	include_directories(${BZIP_INCLUDE_PATH})
endif (BZIP_INCLUDE_PATH)

# Find the directory containing curl/curl.h
find_path(CURL_INCLUDE_PATH curl/curl.h DOC "Directory containing curl/curl.h")
if (CURL_INCLUDE_PATH)
	# Add the curl include directory to the compilers search path 
	include_directories(${CURL_INCLUDE_PATH})
endif (CURL_INCLUDE_PATH)

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

