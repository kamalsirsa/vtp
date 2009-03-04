# Add a library target called minidata
add_library(minidata jpegbase.cpp MiniDatabuf.cpp LocalDatabuf.cpp minidata.cpp pngbase.cpp jpegbase.h LocalDatabuf.h MiniDatabuf.h pngbase.h)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif (ZLIB_FOUND)

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

if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif (GDAL_FOUND)

include_directories(${TERRAIN_SDK_ROOT})
