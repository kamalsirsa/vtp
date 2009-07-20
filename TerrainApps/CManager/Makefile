#
# GNU Makefile:  TerrainApps/CManager
#

VTP_ROOT = $(shell cd ../..; pwd)
EXEC = CManager
include $(VTP_ROOT)/Make.defs

IFLAGS = $(shell $(WX_DIR)/bin/wx-config --cflags) \
	  -I$(TERRAIN_SDK_DIR) \
	  -I$(OSG_INC) \
	  -I$(GDAL_INC)
ifneq ($(OS),Darwin)
IFLAGS += -I$(X_INC)
endif

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
	  $(EXTRA_LDIRS)
ifneq ($(OS),Darwin)
LDIRS += $(X_LIB)
endif

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
	 -lm

ifeq ($(OS),Darwin)
# Support for Mac OS X
OSX_APP = $(OSX_APPS)/$(EXEC).app/Contents/MacOS/$(EXEC)
LIBS   += -F$(FRAMEWORKS) \
	  -lobjc \
	  $(call _VLF,vtosg) $(call _VLF,vtdata) $(call _VLF,xmlhelper) $(call _VLF,unzip) $(call _VLF,vtui) \
	  $(OSG_OPTS) \
	  -framework OpenGL \
	  -L$(FINK_LIB) \
	  -lz -lstdc++ \
	  -Wl,-executable_path -Wl,$(OSX_APPS)/$(EXEC).app/Contents/MacOS \
	  -bind_at_load
else
# Support for all other Unix platforms
LIBS   +=  -lxmlhelper -lunzip -lvtdata -lvtui -lvtosg -lminidata \
	$(MINI_LIB) -lGL $(GLU_LIB) -lX11 $(EXTRA_LIBS) \
	-losg -losgDB -losgUtil -losgText -losgParticle -losgShadow -lOpenThreads
endif

ifeq ($(SUPPORT_NETCDF),1)
	LIBS += -lnetcdf
endif
ifeq ($(SUPPORT_HTTP),1)
	LIBS += -lcurl
endif
ifeq ($(SUPPORT_BZIP2),1)
	LIBS += -lbz2
endif
ifeq ($(SUPPORT_SQUISH),1)
	LIBS += -lsquish
endif

LDIR   +=\
	  $(VTDATA_INST_DIR)             \
	  $(VTUI_INST_DIR)               \
	  $(VTOSG_INST_DIR)              \
	  $(XMLHELPER_INST_DIR)


ifeq ($(OS),Darwin)
all : $(OSX_APP)
else
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
