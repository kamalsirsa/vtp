#
# GNU Makefile:  TerrainSDK/xmlhelper
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)

include $(VTP_ROOT)/TerrainSDK/Make.defs

IFLAGS =

SWITCHES = -DUNIX
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)

CSRC   = $(wildcard *.c)
CXXSRC = $(wildcard *.cpp)
OBJS   = $(CSRC:%.c=%.o) $(CXXSRC:%.cpp=%.o)
LIB    = libxmlhelper.so

all : $(LIB)

$(LIB) : $(OBJS)
	$(LD_SHARED) -o $(LIB) $(OBJS)
#	$(AR) $(LIB) $(OBJS)

clean :
	rm -rf $(OBJS) ii_files so_locations

clobber :
	rm -rf $(OBJS) $(LIB) ii_files so_locations

