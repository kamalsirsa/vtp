#
# GNU Makefile:  TerrainApps/Enviro
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)

include $(VTP_ROOT)/TerrainApps/Make.defs

IFLAGS = $(shell wx-config --cflags) \
	  -I$(TERRAIN_SDK_DIR)       \
	  -I$(OSG_INC)               \
	  -I$(GDAL_INC)              \
	  -I$(TERRAIN_APPS_DIR)/Enviro/wx/bitmap \
	  -I$(TERRAIN_APPS_DIR)/Enviro/wx/icons

LDIRS  = \
	  $(OSG_LIB)                     \
	  $(VTDATA_INST_DIR)             \
	  $(VTUI_INST_DIR)               \
	  $(VTOSG_INST_DIR)              \
	  $(XMLHELPER_INST_DIR)          \
	  $(SHAPE_LIB)                   \
	  $(PNG_LIB)                     \
	  $(GDAL_LIB)                    \
	  $(NETCDF_LIB)                  \
	  $(X_LIB)                       \
	  $(EXTRA_LDIRS)

SWITCHES := $(SWITCHES) -DUNIX -DVTLIB_OSG=1
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)
LDFLAGS  = $(LDIRFLAGS) \
	   $(EXTRA_LDFLAGS)

CXXSRC = $(wildcard *.cpp) \
	 $(filter-out wx/StartupDlg0.cpp, $(wildcard wx/*.cpp))
OBJS   = $(CXXSRC:%.cpp=%.o)
EXEC   = wxEnviro
LIBS   = $(shell wx-config --libs) $(shell wx-config --gl-libs) \
	  -lgdal.1.1 -lvtosg -lvtui -lvtdata -lxmlhelper $(MINI_LIB) \
	  -losgDB -losgText -losgUtil -losg -lnetcdf \
	  $(GLU_LIB) -lGL -lX11 $(EXTRA_LIBS) -lm 

all : $(EXEC)

$(EXEC) : $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

clean :
	rm -rf $(OBJS) $(EXEC).core core ii_files wx/ii_files

clobber :
	rm -rf $(OBJS) $(EXEC).core core ii_files wx/ii_files $(EXEC) $(EXEC).pure

#
# Internationalization steps for Swedish
#

create:
	xgettext -k_ -o Enviro.pot wx/frame.cpp
	mkdir sv
	cp Enviro.pot sv/Enviro.pox

publish:
	cp sv/Enviro.pox sv/Enviro.po
	msgfmt sv/Enviro.po -o sv/Enviro.mo

update:
	xgettext -k_ -o Enviro.pot wx/frame.cpp
	msgmerge sv/Enviro.po Enviro.pot -o sv/Enviro.pox

