#
# GNU Makefile:  TerrainSDK/xmlhelper
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)
LIB_NAME = xmlhelper

include $(VTP_ROOT)/Make.defs

IFLAGS =



ifeq ($(OS), Darwin)
all : $(FRAMEWORK_LIB)
else
all : $(LIB)
endif

$(LIB) : $(OBJS)
	$(LD_SHARED) -o $(LIB) $(XMLHELPER_LINK_OPTS) $(OBJS) 


$(FRAMEWORK_LIB) : $(OBJS) $(FRAMEWORK)
	$(LD_SHARED) -o $(FRAMEWORK_LIB) -install_name @executable_path$(FRAMEWORK_INSTALL_NAME) $(XMLHELPER_LINK_OPTS) $(OBJS) 
	-ln -s $(FRAMEWORK_LIB) $(FRAMEWORK_EXE)

clean :
	rm -rf $(OBJS) ii_files so_locations

clobber :
	rm -rf $(OBJS) $(LIB) ii_files so_locations $(FRAMEWORK)

