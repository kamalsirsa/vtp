# Add a library target called minidata
add_library(minidata jpegbase.cpp LocalDatabuf.cpp minidata.cpp pngbase.cpp jpegbase.h LocalDatabuf.h MiniDatabuf.h pngbase.h)

# Find the directory containing zlib.h
find_path(ZLIB_INCLUDE_PATH zlib.h DOC "Directory containing zlib.h")
# N.B. You cannot use ${xxxx} in an if (or other expression) to test if xxxx has been set
# if CMake cannot find the variable named xxxx it will use the direct value ${xxxx} which will never be a boolean true
# Enclose the include_directories command in a conditional to stop cmake complaing when it has not been set
if (ZLIB_INCLUDE_PATH)
	# Add the zlib include directory to the compilers search path 
	include_directories(${ZLIB_INCLUDE_PATH})
endif (ZLIB_INCLUDE_PATH)

# Find the directory containing mini/mini.h
find_path(MINI_INCLUDE_PATH mini/mini.h DOC "Directory containing mini/mini.h")
if (MINI_INCLUDE_PATH)
	# Add the mini include directory to the compilers search path 
	include_directories(${MINI_INCLUDE_PATH})
endif (MINI_INCLUDE_PATH)

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

include_directories(${TERRAIN_SDK_ROOT})
