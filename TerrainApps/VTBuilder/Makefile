#
# GNU Makefile:  TerrainApps/VTBuilder
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)
EXEC   = VTBuilder
include $(VTP_ROOT)/Make.defs

IFLAGS = $(shell $(WX_DIR)/bin/wx-config --cflags) \
	  -I$(GDAL_INC)              \
	  -I$(TERRAIN_SDK_DIR)       \
	  -I$(TERRAIN_APPS_DIR)/VTBuilder/bitmaps \
	  -I$(TERRAIN_APPS_DIR)/VTBuilder/icons

LDIRS  = \
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
LIBS   = $(shell $(WX_DIR)/bin/wx-config --libs) \
	-lgdal -lm

# Support of Mac OS X
ifeq ($(OS),Darwin)
OSX_APP = $(OSX_APPS)/$(EXEC).app/Contents/MacOS/$(EXEC)
LIBS   += -F$(FRAMEWORKS) \
	  -lobjc -framework Carbon \
	  -losg -losgDB -losgGA -losgText -losgUtil -lOpenThreads \
	  -framework vtdata -framework vtui \
	  -framework xmlhelper \
	  -L$(FINK_LIB) -lnetcdf \
	  -lz

all : $(OSX_APP)

else
# Support for all other Unix platforms
LIBS   +=  -lGL -lX11 $(EXTRA_LIBS)   \
	  -lxmlhelper -lvtdata -lvtui \
	  -lnetcdf                  

LDIRS  += \
	  $(VTDATA_INST_DIR)             \
	  $(VTUI_INST_DIR)               \
          $(XMLHELPER_INST_DIR)

all : $(EXEC)
endif

$(EXEC) : $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

$(OSX_APP) : $(OBJS) $(OSX_APPS)/$(EXEC).app
	$(LD) -o $(OSX_APP) $(OBJS) $(LDFLAGS) $(LIBS)

clean :
	rm -rf $(OBJS) $(EXEC).core core ii_files wx/ii_files

clobber :
	rm -rf $(OBJS) $(EXEC).core core ii_files wx/ii_files $(EXEC) $(EXEC).pure $(OSX_APPS)/$(EXEC).app

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

publish-zh:
	cp zh/VTBuilder.pox zh/VTBuilder.po
	msgfmt zh/VTBuilder.po -o zh/VTBuilder.mo

update-zh:
	msgmerge --no-fuzzy-matching zh/VTBuilder.po VTBuilder.pot -o zh/VTBuilder.pox

