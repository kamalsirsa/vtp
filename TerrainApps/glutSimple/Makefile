#
# GNU Makefile:  TerrainApps/glutSimple
#

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
	 $(UNZIP_INST_DIR)              \
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
	-framework osg -framework osgGA -framework osgUtil \
        -framework osgText -framework osgDB -framework OpenThreads \
	-L$(FINK_LIB) -lnetcdf -lgdal \
	-framework xmlhelper $(MINI_LIB) \
        $(GLUT_LIB) $(GLU_LIB) -lGL -lXmu -lX11 $(EXTRA_LIBS) \
	-framework Carbon -lobjc -lm 

# Use this if you're not using Xcode libraries
#	-lgdal -losgGA -losgDB -losgUtil -losgText -losg -lOpenThreads \

all : $(OSX_APP)

else
LIBS   = -lvtosg -lvtdata -lunzip \
         -lgdal -losgDB -losgUtil -losgText -losg -lOpenThreads \
		 -lxmlhelper $(MINI_LIB) \
         $(GLUT_LIB) $(GLU_LIB) -lGL -lXmu -lX11 $(EXTRA_LIBS) -lcurl -lbz2 -lm 

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
