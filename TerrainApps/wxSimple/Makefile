#
# GNU Makefile:  TerrainApps/wxSimple
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)

include $(VTP_ROOT)/TerrainApps/Make.defs

IFLAGS = $(shell wx-config --cflags) \
	  -I$(TERRAIN_SDK_DIR)       \
	  -I$(OSG_INC)               \
	  -I$(X_INC)

LDIRS  = $(OSG_LIB)                     \
	 $(VTDATA_INST_DIR)             \
	 $(VTOSG_INST_DIR)              \
	 $(PNG_LIB)                     \
	 $(SHAPE_LIB)                   \
	 $(NETCDF_LIB)                  \
	 $(X_LIB)                       \
	 $(EXTRA_LDIRS)

SWITCHES := $(SWITCHES) -DUNIX -DVTLIB_OSG=1
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)
LDFLAGS  = $(LDIRFLAGS) \
	   $(EXTRA_LDFLAGS)

CXXSRC = $(wildcard *.cpp)
OBJS   = $(CXXSRC:%.cpp=%.o)
EXEC   = wxSimple
LIBS   = $(shell wx-config --libs)   \
	 -lwx_gtk_gl                 \
         -lvtosg -lvtdata            \
         -lgdal.1.1 -losgDB -losgUtil -losg -lnetcdf    \
         $(GLU_LIB) -lGL -lX11 $(EXTRA_LIBS) -lm 

all : $(EXEC)

$(EXEC) : $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

clean :
	rm -rf $(OBJS) $(EXEC).core core ii_files

clobber :
	rm -rf $(OBJS) $(EXEC).core core ii_files $(EXEC) $(EXEC).pure
