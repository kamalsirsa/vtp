#
# GNU Makefile:  TerrainApps/VTBuilder
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)

include $(VTP_ROOT)/TerrainApps/Make.defs

IFLAGS = $(shell wx-config --cflags) \
	  -I$(GDAL_INC)              \
	  -I$(TERRAIN_SDK_DIR)       \
	  -I$(TERRAIN_APPS_DIR)/VTBuilder/bitmaps \
	  -I$(TERRAIN_APPS_DIR)/VTBuilder/icons

LDIRS  = \
	  $(VTDATA_INST_DIR)             \
	  $(VTUI_INST_DIR)               \
 	  $(XMLHELPER_INST_DIR)          \
	  $(SHAPE_LIB)                   \
	  $(GDAL_LIB)                    \
	  $(PNG_LIB)                     \
	  $(NETCDF_LIB)                  \
	  $(X_LIB)                       \
	  $(EXTRA_LDIRS)

SWITCHES := $(SWITCHES) -DUNIX
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)
LDFLAGS  = $(LDIRFLAGS) \
	   $(EXTRA_LDFLAGS)

CXXSRC = $(filter-out pre.cpp, $(wildcard *.cpp))
OBJS   = $(CXXSRC:%.cpp=%.o)
EXEC   = VTBuilder
LIBS   = $(shell wx-config --libs)     \
	  $(shell wx-config --gl-libs)     \
	  -lgdal -lvtdata -lvtui       \
	  -lxmlhelper					   \
	  -lnetcdf                         \
	  -lGL -lX11 $(EXTRA_LIBS) -lm 

all : $(EXEC)

$(EXEC) : $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

clean :
	rm -rf $(OBJS) $(EXEC).core core ii_files wx/ii_files

clobber :
	rm -rf $(OBJS) $(EXEC).core core ii_files wx/ii_files $(EXEC) $(EXEC).pure

#
# Internationalization steps (iso-8859-1) for VTBuilder
#

I18N_SOURCES = \
	$(CXXSRC) \
	VTBuilder_wdr.cpp \
	../../TerrainSDK/vtui/vtui_wdr.cpp \
	../../TerrainSDK/vtui/BuildingDlg.cpp

update-pot:
	xgettext --from-code=iso-8859-1 -k_ -o VTBuilder.pot $(I18N_SOURCES)

