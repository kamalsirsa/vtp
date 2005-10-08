#
# GNU Makefile:  TerrainApps/glutSimple
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)
EXEC = glutSimple
include $(VTP_ROOT)/Make.defs

IFLAGS = -I$(X_INC)                 \
         -I$(TERRAIN_SDK_DIR)       \
         -I$(VTLIB_DIR)             \
	 -I$(OSG_INC)               \
	 -I$(GDAL_INC)              \
	 -I$(X_INC)

LDIRS  = $(OSG_LIB)                     \
	 $(VTDATA_INST_DIR)             \
	 $(XMLHELPER_INST_DIR)          \
	 $(XMLHELPER_INST_DIR)          \
	 $(VTOSG_INST_DIR)              \
         $(PNG_LIB)                     \
         $(NETCDF_LIB)                  \
         $(GDAL_LIB)                    \
	 $(X_LIB)                       \
	 $(EXTRA_LDIRS)

SWITCHES := $(SWITCHES) -DUNIX -DVTLIB_OSG=1
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)
LDFLAGS  = $(LDIRFLAGS) $(EXTRA_LDFLAGS)

CXXSRC = $(wildcard *.cpp)
OBJS   = $(CXXSRC:%.cpp=%.o)

ifeq ($(OS),Darwin)
OSX_APP = $(OSX_APPS)/$(EXEC).app/Contents/MacOS/$(EXEC)
LIBS   = -bind_at_load -F$(FRAMEWORKS) \
	-framework vtosg -framework vtdata \
	-lgdal -losgGA -losgDB -losgUtil -losgText -losg -lOpenThreads \
	-L$(FINK_LIB) -lnetcdf \
	-framework xmlhelper $(MINI_LIB) \
        $(GLUT_LIB) $(GLU_LIB) -lGL -lXmu -lX11 $(EXTRA_LIBS) \
	-framework Carbon -lobjc -lm 

all : $(OSX_APP)

else
LIBS   = -lvtosg -lvtdata \
         -lgdal -losgDB -losgUtil -losgText -losg -lnetcdf \
		 -lxmlhelper $(MINI_LIB) \
         $(GLUT_LIB) $(GLU_LIB) -lGL -lXmu -lX11 $(EXTRA_LIBS) -lm 

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
