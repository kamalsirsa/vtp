# Add a library target called xmlhelper
add_library(xmlhelper easyxml.cpp exception.cpp hashtable.c widestring.cpp xmlparse.c xmlrole.c xmltok.c easyxml.hpp exception.hpp widestring.h)

# Make the debug configuration have a postfix on the output target
set_target_properties(xmlhelper PROPERTIES DEBUG_POSTFIX "d")

# Find the directory containing zlib.h
find_path(ZLIB_INCLUDE_PATH zlib.h DOC "Directory containing zlib.h")

# N.B. You cannot use ${xxxx} in an if (or other expression) to test if xxxx has been set
# if CMake cannot find the variable named xxxx it will use the direct value ${xxxx} which will never be a boolean true
# Enclose the include_directories command in a conditional to stop cmake complaing when it has not been set
if (ZLIB_INCLUDE_PATH)
	# Add the zlib include directory to the compilers search path 
	include_directories(${ZLIB_INCLUDE_PATH})
endif (ZLIB_INCLUDE_PATH)

# Windows specific stuff
if (WIN32)
	set_property(TARGET xmlhelper APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
endif (WIN32)