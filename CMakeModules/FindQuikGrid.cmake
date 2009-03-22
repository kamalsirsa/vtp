# Defines
# QUIKGRID_FOUND
# QUIKGRID_INCLUDE_DIR
# QUIKGRID_LIBRARIES

set(QuikGrid_FIND_DEBUG FALSE CACHE BOOL "Also search for the debug version of the QuikGrid library")
mark_as_advanced(QuikGrid_FIND_DEBUG)

if(QuikGrid_FIND_DEBUG)
	if(QUIKGRID_INCLUDE_DIR AND QUIKGRID_LIBRARY AND QUIKGRID_LIBRARY_DEBUG)
		set(QuikGrid_FIND_QUIETLY TRUE)
	endif(QUIKGRID_INCLUDE_DIR AND QUIKGRID_LIBRARY AND QUIKGRID_LIBRARY_DEBUG)
else(QuikGrid_FIND_DEBUG)
	if(QUIKGRID_INCLUDE_DIR AND QUIKGRID_LIBRARY)
		set(QuikGrid_FIND_QUIETLY TRUE)
	endif(QUIKGRID_INCLUDE_DIR AND QUIKGRID_LIBRARY)
endif(QuikGrid_FIND_DEBUG)

find_path(QUIKGRID_INCLUDE_DIR surfgrid.h DOC "Directory containing surfgrid.h")

find_library(QUIKGRID_LIBRARY NAMES QuikGrid DOC "Path to QuikGrid library")
if(QuikGrid_FIND_DEBUG)
	find_library(QUIKGRID_LIBRARY_DEBUG NAMES QuikGridd DOC "Path to QuikGridd library")
endif(QuikGrid_FIND_DEBUG)

# handle the QUIETLY and REQUIRED arguments and set QUIKGRID_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QuikGrid DEFAULT_MSG QUIKGRID_LIBRARY QUIKGRID_INCLUDE_DIR)

if(QUIKGRID_FOUND)
	if(QuikGrid_FIND_DEBUG)
		if(NOT QUIKGRID_LIBRARY_DEBUG)
			set(QUIKGRID_LIBRARY_DEBUG ${QUIKGRID_LIBRARY})
		endif(NOT QUIKGRID_LIBRARY_DEBUG)
		set(QUIKGRID_LIBRARIES debug ${QUIKGRID_LIBRARY_DEBUG} optimized ${QUIKGRID_LIBRARY})
	else(QuikGrid_FIND_DEBUG)
		set(QUIKGRID_LIBRARIES ${QUIKGRID_LIBRARY})
	endif(QuikGrid_FIND_DEBUG)
else(QUIKGRID_FOUND)
   SET(QUIKGRID_LIBRARIES )
endif(QUIKGRID_FOUND)

