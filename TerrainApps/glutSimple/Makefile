#
# GNU Makefile:  TerrainApps/glutSimple
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)

include $(VTP_ROOT)/TerrainApps/Make.defs

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
EXEC   = glutSimple
LIBS   = -lvtosg -lvtdata \
         -lgdal -losgDB -losgUtil -losgText -losg -lnetcdf \
		 -lxmlhelper $(MINI_LIB) \
         $(GLUT_LIB) $(GLU_LIB) -lGL -lXmu -lX11 $(EXTRA_LIBS) -lm 

all : $(EXEC)

$(EXEC) : $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

clean :
	rm -rf $(OBJS) $(EXEC).core core ii_files

clobber :
	rm -rf $(OBJS) $(EXEC).core core ii_files $(EXEC) $(EXEC).pure
