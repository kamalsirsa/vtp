#
# GNU Makefile:  TerrainSDK/vtui
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)

include $(VTP_ROOT)/TerrainSDK/Make.defs

IFLAGS = $(shell wx-config --cflags) \
	-I$(TERRAIN_SDK_DIR)

SWITCHES = -DUNIX
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)

CSRC   = $(wildcard *.c)
CXXSRC = $(wildcard *.cpp) $(wildcard boost/*.cpp)
OBJS   = $(CSRC:%.c=%.o) $(CXXSRC:%.cpp=%.o)
LIB    = libvtui.so

all : $(LIB)

$(LIB) : $(OBJS)
	$(LD_SHARED) -o $(LIB) $(OBJS)
#	$(AR) $(LIB) $(OBJS)

clean :
	rm -rf $(OBJS) ii_files so_locations

clobber :
	rm -rf $(OBJS) $(LIB) ii_files so_locations
