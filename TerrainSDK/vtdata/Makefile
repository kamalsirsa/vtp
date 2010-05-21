#
# GNU Makefile:  TerrainSDK/vtdata
#

VTP_ROOT = $(shell cd ../..; pwd)
LIB_NAME = libvtdata

include $(VTP_ROOT)/Make.defs

IFLAGS = -I./shapelib \
	 -I$(NETCDF_INC) \
	 -I$(GDAL_INC) \
	 -I$(QUIKGRID_INC) \
	 -I$(VTP_ROOT)/TerrainSDK/

HEADERS = $(CHEAD) $(CXXHEAD) ./shapelib

OBJS := $(OBJS) triangle/triangle.o

ifeq ($(OS), Darwin)
all : $(OSX_LIB_TARGET)
else
all : $(LIB)
endif

$(LIB) : $(OBJS)
	$(LD_SHARED) -o $(LIB) $(VTDATA_LINK_OPTS) $(OBJS) 

$(FRAMEWORK_LIB) : $(OBJS) $(FRAMEWORK)
	$(LD_SHARED) -o $(FRAMEWORK_LIB) -install_name $(FRAMEWORK_INSTALL_NAME) $(VTDATA_LINK_OPTS) $(OBJS) 
	@-(cd $(shell dirname $(FRAMEWORK_EXE)) && ln -sf Versions/A/$(LIB_NAME) $(LIB_NAME))

clean :
	rm -rf $(OBJS) ii_files so_locations

clobber :
	rm -rf $(OBJS) $(LIB) ii_files so_locations $(FRAMEWORK)

install :
	cp -rfp $(LIB) $(VTP_INSTALL_DIR)/lib
	mkdir -p $(VTP_INSTALL_DIR)/include/vtdata
	cp -rfp $(HEADERS) $(VTP_INSTALL_DIR)/include/vtdata
