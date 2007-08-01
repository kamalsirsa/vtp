#
# GNU Makefile:  TerrainApps/CManager
#

VTP_ROOT = $(shell cd ../..; pwd)
EXEC = CManager
include $(VTP_ROOT)/Make.defs

IFLAGS = $(shell $(WX_DIR)/bin/wx-config --cflags) \
	  -I$(TERRAIN_SDK_DIR)

LDIRS  = \
	  $(VTDATA_INST_DIR)     \
	  $(VTOSG_INST_DIR)      \
	  $(VTUI_INST_DIR)       \
	  $(XMLHELPER_INST_DIR)  \
	  $(UNZIP_INST_DIR)      \
	  $(OSG_LIB)             \
	  $(SHAPE_LIB)           \
	  $(PNG_LIB)             \
	  $(GDAL_LIB)            \
	  $(X_LIB)               \
	  $(EXTRA_LDIRS)

SWITCHES := $(SWITCHES) -DUNIX -DVTLIB_OSG=1
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)
LDFLAGS  = $(LDIRFLAGS) \
	   $(EXTRA_LDFLAGS)

CXXSRC := $(filter-out pre.cpp, $(wildcard *.cpp))
CXXSRC = $(wildcard *.cpp) \
	 $(filter-out wx/StartupDlg0.cpp, $(wildcard wx/*.cpp))
OBJS   = $(CXXSRC:%.cpp=%.o)

LIBS   = $(shell $(WX_DIR)/bin/wx-config --libs) \
	 $(shell $(WX_DIR)/bin/wx-config --gl-libs) \
	 -lgdal \
	 -lbz2 -lcurl -lm $(EXTRA_LIBS)

# Support of Mac OS X
ifeq ($(OS),Darwin)
OSX_APP = $(OSX_APPS)/$(EXEC).app/Contents/MacOS/$(EXEC)
LIBS   += -F$(FRAMEWORKS) \
	  -lobjc -framework Carbon \
	  -framework vtdata -framework vtui -framework vtosg \
	  -framework xmlhelper -framework unzip \
 	  -framework osg -framework osgDB -framework osgUtil \
	  -framework osgText -framework OpenThreads \
	  -framework OpenGL \
	  -L$(FINK_LIB) -lnetcdf \
	  -lX11 -L$(X11BASE)/lib \
	  -lz -lstdc++ \
	  -Wl,-executable_path -Wl,$(OSX_APPS)/$(EXEC).app/Contents/MacOS \
	  -bind_at_load

all : $(OSX_APP)

else
# Support for all other Unix platforms
LIBS   +=  -lxmlhelper -lunzip -lvtdata -lvtui -lvtosg \
	$(MINI_LIB) -lGL $(GLU_LIB) -lX11 \
	-losg -losgDB -losgUtil -losgText -lOpenThreads                

LDIR   +=\
	  $(VTDATA_INST_DIR)             \
	  $(VTUI_INST_DIR)               \
	  $(VTOSG_INST_DIR)              \
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
