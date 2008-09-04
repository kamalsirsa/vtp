#
# GNU Makefile:  TerrainSDK/minidata
#

VTP_ROOT = $(shell cd ../..; pwd)
LIB_NAME = libminidata

include $(VTP_ROOT)/Make.defs

OBJS := $(OBJS)

IFLAGS = -I$(GDAL_INC) \
	-I$(MINI_INC) \
	-I$(TERRAIN_SDK_DIR)


ifeq ($(OS), Darwin)
all : $(FRAMEWORK_LIB)
else
all : $(LIB)
endif

$(LIB) : $(OBJS)
	$(LD_SHARED) -o $(LIB) $(MINIDATA_LINK_OPTS) $(OBJS) 


$(FRAMEWORK_LIB) : $(OBJS) $(FRAMEWORK)
	$(LD_SHARED) -o $(FRAMEWORK_LIB) -install_name $(FRAMEWORK_INSTALL_NAME) $(MINIDATA_LINK_OPTS) $(OBJS) 
	@-(cd $(shell dirname $(FRAMEWORK_EXE)) && ln -sf Versions/A/$(LIB_NAME) $(LIB_NAME))

clean :
	rm -rf $(OBJS) ii_files so_locations

clobber :
	rm -rf $(OBJS) $(LIB) ii_files so_locations $(FRAMEWORK)

install :
	cp -rfp $(LIB) $(VTP_INSTALL_DIR)/lib
