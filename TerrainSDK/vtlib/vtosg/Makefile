#
# GNU Makefile:  TerrainSDK/vtlib/vtosg
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../../..; pwd)

include $(VTP_ROOT)/TerrainSDK/Make.defs

IFLAGS = -I$(TERRAIN_SDK_DIR) \
         -I$(VTLIB_DIR)       \
         -I$(PNG_INC)         \
	 -I$(OSG_INC)         \
	 -I$(MINI_INC)        \
	 -I$(GDAL_INC)        \
	 -I$(APIS_DIR)        \
	 -I$(X_INC)

SWITCHES = -DUNIX -DVTLIB_OSG=1
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)

CXXSRC = $(wildcard *.cpp) \
         $(wildcard ../core/*.cpp)
OBJS   = $(CXXSRC:%.cpp=%.o)
LIB    = libvtosg.so

all : $(LIB)

$(LIB) : $(OBJS)
	$(LD_SHARED) -o $(LIB) $(OBJS)
#	$(AR) $(LIB) $(OBJS)

clean :
	rm -rf $(OBJS) ii_files so_locations

clobber :
	rm -rf $(OBJS) $(LIB) ii_files ../core/ii_files so_locations
