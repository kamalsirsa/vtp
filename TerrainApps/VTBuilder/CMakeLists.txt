# Add a library target called builderlib
set(BUILDERLIB_SOURCE_FILES
	Builder.cpp BuilderView.cpp ChunkDlg.cpp ElevLayer.cpp Export.cpp ExtentDlg.cpp Helper.cpp ImageLayer.cpp
	Import.cpp ImportPointDlg.cpp ImportStructDlg.cpp ImportStructDlgOGR.cpp ImportVegDlg.cpp Layer.cpp NodeDlg.cpp RawDlg.cpp
	RawLayer.cpp RenderOptionsDlg.cpp ResampleDlg.cpp RoadDlg.cpp RoadLayer.cpp RoadMapEdit.cpp RoadMapIO.cpp SampleImageDlg.cpp
	ScaledView.cpp StructLayer.cpp TileDlg.cpp UtilityLayer.cpp VegFieldsDlg.cpp VegLayer.cpp vtBitmap.cpp VTBuilder_wdr.cpp
	vtImage.cpp WaterLayer.cpp)
set(BUILDERLIB_SOURCE_FILES_NO_PCH
	ElevDrawOptions.cpp)
set(BUILDERLIB_HEADER_FILES
	Builder.h BuilderView.h ChunkDlg.h ElevDrawOptions.h ElevLayer.h ExtentDlg.h Helper.h ImageLayer.h ImportPointDlg.h ImportStructDlg.h
	ImportStructDlgOGR.h ImportVegDlg.h Layer.h NodeDlg.h Options.h RawDlg.h RawLayer.h RenderOptionsDlg.h ResampleDlg.h RoadDlg.h
	RoadLayer.h RoadMapEdit.h SampleImageDlg.h ScaledView.h StructLayer.h TileDlg.h UtilityLayer.h VegFieldsDlg.h VegLayer.h
	vtBitmap.h VTBuilder_wdr.h vtImage.h WaterLayer.h)
if(MSVC)
	add_library(builderlib ${BUILDERLIB_SOURCE_FILES} ${BUILDERLIB_SOURCE_FILES_NO_PCH} ${BUILDERLIB_HEADER_FILES} pre.cpp)
	set_source_files_properties(${BUILDERLIB_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
	set_source_files_properties(pre.cpp PROPERTIES COMPILE_FLAGS /Ycwx/wxprec.h)
else(MSVC)
	add_library(builderlib ${BUILDERLIB_SOURCE_FILES} ${BUILDERLIB_HEADER_FILES})
endif(MSVC)

# Specify common preprocessor definitions for this target
set_property(TARGET builderlib APPEND PROPERTY COMPILE_DEFINITIONS SUPPORT_TRANSIT=0 USE_OPENGL=1)

# Windows specific stuff
if (WIN32)
	set_property(TARGET builderlib APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE _UNICODE UNICODE WXUSINGDLL=1)
endif (WIN32)

# Specify debug preprocessor definitions for this target
set_property(TARGET builderlib APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG VTDEBUG)

set(VTBUILDER_SOURCE_FILES
	App.cpp Frame.cpp FrameMenus.cpp Geocode.cpp RoadFix.cpp Special.cpp StatusBar.cpp TransitLayer.cpp TreeView.cpp)
set(VTBUILDER_DIALOG_SOURCE_FILES
	DistanceDlg2d.cpp DistribVegDlg.cpp FeatInfoDlg.cpp GenGridDlg.cpp GeocodeDlg.cpp ImageMapDlg.cpp LayerPropDlg.cpp
	MapServerDlg.cpp MatchDlg.cpp NodeDlg.cpp OptionsDlg.cpp PrefDlg.cpp RenderDlg.cpp SelectDlg.cpp
	TSDlg.cpp VegDlg.cpp)
set(VTBUILDER_HEADER_FILES
	app.h DistanceDlg2d.h DistribVegDlg.h FeatInfoDlg.h FileFilters.h Frame.h GenGridDlg.h GeocodeDlg.h ImageMapDlg.h
	LayerPropDlg.h MapServerDlg.h MatchDlg.h MenuEnum.h OptionsDlg.h PrefDlg.h RenderDlg.h RoadMapEdit.h
	SelectDlg.h StatusBar.h TransitLayer.h TreeView.h TSDlg.h VegDlg.h VegPointOptions.h)
set(VTBUILDER_RC_FILES
	VTBuilder.rc)
if(MSVC)
	add_executable(vtbuilder WIN32 ${VTBUILDER_SOURCE_FILES} ${VTBUILDER_DIALOG_SOURCE_FILES} ${VTBUILDER_HEADER_FILES} pre.cpp
		${VTBUILDER_RC_FILES})
	set_source_files_properties(${VTBUILDER_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
	# already handled pre.cpp earlier on
else(MSVC)
	add_executable(vtbuilder WIN32 ${VTBUILDER_SOURCE_FILES} ${VTBUILDER_DIALOG_SOURCE_FILES} ${VTBUILDER_HEADER_FILES}
		${VTBUILDER_RC_FILES})
endif(MSVC)

# Internal dependencies for this target
add_dependencies(vtbuilder builderlib vtdata vtui xmlhelper minidata unzip)

# Specify common preprocessor definitions for this target
set_property(TARGET vtbuilder APPEND PROPERTY COMPILE_DEFINITIONS SUPPORT_TRANSIT=0 USE_OPENGL=1)

# Windows specific stuff
if (WIN32)
	set_property(TARGET vtbuilder APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE _UNICODE UNICODE WXUSINGDLL=1)
	set_property(TARGET vtbuilder APPEND PROPERTY LINK_FLAGS_DEBUG /NODEFAULTLIB:msvcrt;wxpngd;wxtiffd;wxjpegd;wxzlibd)
	set_property(TARGET vtbuilder APPEND PROPERTY LINK_FLAGS /NODEFAULTLIB:wxpng;wxtiff;wxjpeg;wxzlib)
endif (WIN32)

# Specify debug preprocessor definitions for this target
set_property(TARGET vtbuilder APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG VTDEBUG)

# External libraries for this target
if(wxWidgets_FOUND)
	target_link_libraries(vtbuilder ${wxWidgets_LIBRARIES})
endif(wxWidgets_FOUND)
if(GDAL_FOUND)
	target_link_libraries(vtbuilder ${GDAL_LIBRARY})
endif (GDAL_FOUND)
if(BZIP2_FOUND)
	target_link_libraries(vtbuilder ${BZIP2_LIBRARIES})
endif(BZIP2_FOUND)
if(CURL_FOUND)
	target_link_libraries(vtbuilder ${CURL_LIBRARIES})
endif(CURL_FOUND)
if(ZLIB_FOUND)
	target_link_libraries(vtbuilder ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)
if(JPEG_FOUND)
	target_link_libraries(vtbuilder ${JPEG_LIBRARY})
endif(JPEG_FOUND)
if(PNG_FOUND)
	target_link_libraries(vtbuilder ${PNG_LIBRARY})
endif(PNG_FOUND)
find_library(SQUISH_LIBRARY squish DOC "Path to squish library")
if(SQUISH_LIBRARY)
	target_link_libraries(vtbuilder ${SQUISH_LIBRARY})
endif(SQUISH_LIBRARY)
find_library(QUIKGRID_LIBRARY quikgrid DOC "Path to quikgrid library")
if(QUIKGRID_LIBRARY)
	target_link_libraries(vtbuilder ${QUIKGRID_LIBRARY})
endif(QUIKGRID_LIBRARY)
find_library(MINI_LIBRARY mini DOC "Path to libmini")
if(MINI_LIBRARY)
	target_link_libraries(vtbuilder ${MINI_LIBRARY})
endif(MINI_LIBRARY)
find_library(NETCDF_LIBRARY netcdf DOC "Path to netcdf library")
if(NETCDF_LIBRARY)
	target_link_libraries(vtbuilder ${NETCDF_LIBRARY})
endif(NETCDF_LIBRARY)

# Set up include directories for all targets at this level
include_directories(${TERRAIN_SDK_ROOT})

if(wxWidgets_FOUND)
	include_directories(${wxWidgets_INCLUDE_DIRS})
endif(wxWidgets_FOUND)

if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif(GDAL_FOUND)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

# Find the directory containing mini/mini.h
find_path(MINI_INCLUDE_PATH mini/mini.h DOC "Directory containing mini/mini.h")
if (MINI_INCLUDE_PATH)
	# Add the mini include directory to the compilers search path 
	include_directories(${MINI_INCLUDE_PATH})
endif (MINI_INCLUDE_PATH)

if(PNG_FOUND)
	# Add the png include directory to the compilers search path 
	include_directories(${PNG_PNG_INCLUDE_DIR})
endif(PNG_FOUND)

# Find the directory containing surfgrid.h
find_path(QUIKGRID_INCLUDE_PATH surfgrid.h DOC "Directory containing surfgrid.h")
if (QUIKGRID_INCLUDE_PATH)
	# Add the QuikGrid include directory to the compilers search path 
	include_directories(${QUIKGRID_INCLUDE_PATH})
endif (QUIKGRID_INCLUDE_PATH)

if(JPEG_FOUND)
	include_directories(${JPEG_INCLUDE_DIR})
endif(JPEG_FOUND)

# Find the directory containing squish.h
find_path(SQUISH_INCLUDE_PATH squish.h DOC "Directory containing squish.h")
if (SQUISH_INCLUDE_PATH)
	# Add the squish include directory to the compilers search path 
	include_directories(${SQUISH_INCLUDE_PATH})
endif (SQUISH_INCLUDE_PATH)

# Find the directory containing GL/glext.h
find_path(GLEXT_INCLUDE_PATH GL/glext.h DOC "Directory containing GL/glext.h")
if (GLEXT_INCLUDE_PATH)
	# Add the glext include directory to the compilers search path 
	include_directories(${GLEXT_INCLUDE_PATH})
endif (GLEXT_INCLUDE_PATH)

