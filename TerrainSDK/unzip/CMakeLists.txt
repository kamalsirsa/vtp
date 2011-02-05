# Add a library target called unzip
add_library(unzip ioapi.c iowin32.c unzip.c ioapi.h iowin32.h unzip.h)

# Make the debug configuration have a postfix on the output target
set_target_properties(unzip PROPERTIES DEBUG_POSTFIX "d")

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

# Specify preprocessor definitions for this target
set_property(TARGET unzip APPEND PROPERTY COMPILE_DEFINITIONS NOUNCRYPT)

# Windows specific stuff
if (WIN32)
	set_property(TARGET unzip APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
endif (WIN32)
