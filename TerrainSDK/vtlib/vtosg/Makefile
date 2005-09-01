#
# GNU Makefile:  TerrainSDK/vtlib/vtosg
#

VTP_ROOT = $(shell cd ../../..; pwd)
LIB_NAME = libvtosg

include $(VTP_ROOT)/Make.defs

SWITCHES += -DVTLIB_OSG=1
override CXXSRC = $(wildcard *.cpp) $(wildcard ../core/*.cpp) 
override OBJS = $(CXXSRC:%.cpp=%.o)
HEADERS += $(wildcard ../core/*.h) $(wildcard ../core/*.hpp)

IFLAGS = -I$(TERRAIN_SDK_DIR) \
         -I$(VTLIB_DIR)       \
         -I$(PNG_INC)         \
	 -I$(OSG_INC)         \
	 -I$(MINI_INC)        \
	 -I$(GDAL_INC)        \
	 -I$(APIS_DIR)        \
	 -I$(X_INC)


ifeq ($(OS), Darwin)
all : $(FRAMEWORK_LIB)
else
all : $(LIB)
endif

$(LIB) : $(OBJS)
	$(LD_SHARED) -o $(LIB) $(VTOSG_LINK_OPTS) $(OBJS) 


$(FRAMEWORK_LIB) : $(OBJS) $(FRAMEWORK)
	$(LD_SHARED) -o $(FRAMEWORK_LIB) -install_name @executable_path$(FRAMEWORK_INSTALL_NAME) $(VTOSG_LINK_OPTS) $(OBJS) 
	-ln -s $(FRAMEWORK_LIB) $(FRAMEWORK_EXE)

clean :
	rm -rf $(OBJS) ii_files so_locations

clobber :
	rm -rf $(OBJS) $(LIB) ii_files so_locations $(FRAMEWORK)

