#
# GNU Makefile:  TerrainSDK/vtdata
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)

include $(VTP_ROOT)/TerrainSDK/Make.defs

IFLAGS = -I./shapelib \
         -I$(NETCDF_INC) \
         -I$(GDAL_INC) \
		 -I$(VTP_ROOT)/TerrainSDK/

#SWITCHES = -DUNIX -DSUPPORT_NETCDF=1 -DSUPPORT_SDTS=1
SWITCHES = -DUNIX -DSUPPORT_NETCDF=1
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)

CSRC   = $(wildcard *.c)
CXXSRC = $(wildcard *.cpp) $(wildcard boost/*.cpp)
OBJS   = $(CSRC:%.c=%.o) $(CXXSRC:%.cpp=%.o)
LIB    = libvtdata.so

all : $(LIB)

$(LIB) : $(OBJS)
	$(LD_SHARED) -o $(LIB) $(OBJS)
#	$(AR) $(LIB) $(OBJS)

clean :
	rm -rf $(OBJS) ii_files so_locations

clobber :
	rm -rf $(OBJS) $(LIB) ii_files so_locations
