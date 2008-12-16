#
# GNU Makefile:  TerrainApps/wxSimple
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)
EXEC = wxSimple
include $(VTP_ROOT)/Make.defs

IFLAGS = $(shell $(WX_DIR)/bin/wx-config --cflags) \
	 -I$(TERRAIN_SDK_DIR)  \
	 -I$(OSG_INC)          \
	 -I$(GDAL_INC)
ifneq ($(OS),Darwin)
IFLAGS += -I$(X_INC)
endif

LDIRS  = $(OSG_LIB)            \
	 $(PNG_LIB)            \
	 $(SHAPE_LIB)          \
	 $(NETCDF_LIB)         \
	 $(GDAL_LIB)           \
	 $(EXTRA_LDIRS)
ifneq ($(OS),Darwin)
LDIRS += $(X_LIB)
endif

SWITCHES := $(SWITCHES) -DUNIX -DVTLIB_OSG=1
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)
LDFLAGS  = $(LDIRFLAGS) \
	   $(EXTRA_LDFLAGS)

CXXSRC = $(wildcard *.cpp)
OBJS   = $(CXXSRC:%.cpp=%.o)

ifeq ($(OS),Darwin)
OSX_APP = $(OSX_APPS)/$(EXEC).app/Contents/MacOS/$(EXEC)
LIBS = $(shell $(WX_DIR)/bin/wx-config --libs) \
	$(shell $(WX_DIR)/bin/wx-config --gl-libs) \
	-bind_at_load -F$(FRAMEWORKS) \
	-framework vtosg -framework vtdata \
	$(OSG_OPTS) \
	-L$(FINK_LIB) -lgdal \
	-framework xmlhelper -framework unzip $(MINI_LIB) \
        $(GLU_LIB) \
	-lobjc -lm \
	-Wl,-executable_path -Wl,$(OSX_APPS)/$(EXEC).app/Contents/MacOS
else
LIBS = $(shell $(WX_DIR)/bin/wx-config --libs) $(shell $(WX_DIR)/bin/wx-config --gl-libs) \
	 -lvtosg -lvtdata -lxmlhelper -lunzip -lminidata \
	 -lgdal -losg -losgDB -losgUtil -losgText -losgParticle -losgShadow -lOpenThreads \
	  $(MINI_LIB) $(GLU_LIB) -lGL -lXmu -lX11 $(EXTRA_LIBS) -lm
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

LDIRS += \
	 $(XMLHELPER_INST_DIR)      \
	 $(UNZIP_INST_DIR)          \
	 $(VTDATA_INST_DIR)         \
	 $(VTOSG_INST_DIR)


ifeq ($(OS),Darwin)
all : $(OSX_APP)
else
all : $(EXEC)
endif

$(EXEC) : $(OBJS)
	$(LD) -o $(EXEC) $(OBJS) $(LDFLAGS) $(LIBS)

$(OSX_APP) : $(OBJS) $(OSX_APPS)/$(EXEC).app
	$(LD) -o $(OSX_APP) $(OBJS) $(LDFLAGS) $(LIBS)

clean :
	rm -rf $(OBJS) $(EXEC).core core ii_files

clobber :
	rm -rf $(OBJS) $(EXEC).core core ii_files $(EXEC) $(EXEC).pure $(OSX_APPS)/$(EXEC).app
