# Add a library target called xmlhelper
add_library(xmlhelper easyxml.cpp exception.cpp hashtable.c widestring.cpp xmlparse.c xmlrole.c xmltok.c easyxml.hpp exception.hpp widestring.h)

# Make the debug configuration have a postfix on the output target
set_target_properties(xmlhelper PROPERTIES DEBUG_POSTFIX "d")

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

# Windows specific stuff
if (WIN32)
	set_property(TARGET xmlhelper APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
endif (WIN32)