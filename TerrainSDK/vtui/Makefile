#
# GNU Makefile:  TerrainSDK/vtui
#

VTP_ROOT = $(shell cd ../..; pwd)
LIB_NAME = libvtui

include $(VTP_ROOT)/Make.defs

# HeightDlg is not built
OBJS := $(filter-out HeightDlg.o,$(OBJS))

IFLAGS = $(shell $(WX_DIR)/bin/wx-config --cflags) \
	-I$(GDAL_INC) \
	-I$(TERRAIN_SDK_DIR)


ifeq ($(OS), Darwin)
all : $(FRAMEWORK_LIB)
else
all : $(LIB)
endif

$(LIB) : $(OBJS)
	$(LD_SHARED) -o $(LIB) $(VTUI_LINK_OPTS) $(OBJS) 


$(FRAMEWORK_LIB) : $(OBJS) $(FRAMEWORK)
	$(LD_SHARED) -o $(FRAMEWORK_LIB) -install_name @executable_path$(FRAMEWORK_INSTALL_NAME) $(VTUI_LINK_OPTS) $(OBJS) 
	-ln -s $(FRAMEWORK_LIB) $(FRAMEWORK_EXE)

clean :
	rm -rf $(OBJS) ii_files so_locations

clobber :
	rm -rf $(OBJS) $(LIB) ii_files so_locations $(FRAMEWORK)

