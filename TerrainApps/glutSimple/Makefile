#
# GNU Makefile:  TerrainApps/glutSimple
#

VTP_ROOT = $(shell cd ../..; pwd)
EXEC = glutSimple
include $(VTP_ROOT)/Make.defs

IFLAGS = -I$(TERRAIN_SDK_DIR)       \
         -I$(VTLIB_DIR)             \
         -I$(OSG_INC)               \
         -I$(GDAL_INC)
ifneq ($(OS),Darwin)
IFLAGS += -I$(X_INC)
endif

LDIRS  = $(OSG_LIB)                \
	 $(VTDATA_INST_DIR)        \
	 $(XMLHELPER_INST_DIR)     \
	 $(UNZIP_INST_DIR)         \
	 $(VTOSG_INST_DIR)         \
	 $(PNG_LIB)                \
	 $(NETCDF_LIB)             \
	 $(GDAL_LIB)               \
	 $(EXTRA_LDIRS)
ifneq ($(OS),Darwin)
LDIRS += $(X_LIB)
endif

SWITCHES := $(SWITCHES) -DUNIX -DVTLIB_OSG=1
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)
LDFLAGS  = $(LDIRFLAGS) $(EXTRA_LDFLAGS)

CXXSRC = $(wildcard *.cpp)
OBJS   = $(CXXSRC:%.cpp=%.o)

ifeq ($(OS),Darwin)
OSX_APP = $(OSX_APPS)/$(EXEC).app/Contents/MacOS/$(EXEC)
LIBS   = -bind_at_load -F$(FRAMEWORKS) \
	 $(call _VLF,vtosg) $(call _VLF,vtdata) $(call _VLF,minidata) $(call _VLF,xmlhelper) $(call _VLF,unzip) \
	$(OSG_OPTS) \
	-L$(FINK_LIB) -lgdal \
	$(MINI_LIB) \
        $(GLUT_LIB) $(GLU_LIB) \
	-lobjc -lm
else
LIBS  = -lvtosg -lvtdata -lunzip -lminidata \
        -lgdal -losg -losgDB -losgUtil -losgText -losgParticle -losgShadow -lOpenThreads \
		-lxmlhelper $(MINI_LIB) \
         $(GLUT_LIB) $(GLU_LIB) -lGL -lXmu -lX11 $(EXTRA_LIBS) -lm
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
	rm -rf $(OBJS) $(EXEC).core core ii_files

clobber :
	rm -rf $(OBJS) $(EXEC).core core ii_files $(EXEC) $(EXEC).pure $(OSX_APPS)/$(EXEC).app
