#
# GNU Makefile:  TerrainApps/wxSimple
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)

include $(VTP_ROOT)/TerrainApps/Make.defs

IFLAGS = $(shell $(WX_DIR)/bin/wx-config --cflags) \
	  -I$(TERRAIN_SDK_DIR)       \
	  -I$(GDAL_INC)              \
	  -I$(OSG_INC)               \
	  -I$(X_INC)

LDIRS  = $(OSG_LIB)          \
	 $(VTDATA_INST_DIR)      \
	 $(VTOSG_INST_DIR)       \
	 $(XMLHELPER_INST_DIR)   \
	 $(PNG_LIB)              \
	 $(SHAPE_LIB)            \
	 $(NETCDF_LIB)           \
	 $(GDAL_LIB)             \
	 $(X_LIB)                \
	 $(EXTRA_LDIRS)

SWITCHES := $(SWITCHES) -DUNIX -DVTLIB_OSG=1
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)
LDFLAGS  = $(LDIRFLAGS) \
	   $(EXTRA_LDFLAGS)

CXXSRC = $(wildcard *.cpp)
OBJS   = $(CXXSRC:%.cpp=%.o)
EXEC   = wxSimple
LIBS   = $(shell $(WX_DIR)/bin/wx-config --libs) $(shell $(WX_DIR)/bin/wx-config --gl-libs) \
	-F$(VTP_ROOT)/VTerrain_OSX/Frameworks \
	-framework vtosg -framework vtdata \
        -framework xmlhelper $(MINI_LIB)   \
         -lgdal -losgGA -losgDB -losgText -losgUtil -losg -lOpenThreads \
	-L/sw/lib -lnetcdf    \
         $(GLU_LIB) -lX11 $(EXTRA_LIBS) -lobjc -lm 

all : $(EXEC)

$(EXEC) : $(OBJS)
	-mkdir -p $(VTP_ROOT)/VTerrain_OSX/$(EXEC).app/Contents/MacOS
	-mkdir -p $(VTP_ROOT)/VTerrain_OSX/$(EXEC).app/Contents/Resources
	$(LD) -o $(VTP_ROOT)/VTerrain_OSX/$(EXEC).app/Contents/MacOS/$(EXEC) $(OBJS) $(LDFLAGS) $(LIBS)
	cp $(EXEC).plist $(VTP_ROOT)/VTerrain_OSX/$(EXEC).app/Contents/Info.plist
	echo -n "APPL????" > $(VTP_ROOT)/VTerrain_OSX/$(EXEC).app/Contents/PkgInfo
	cp /usr/local/lib/libwx_mac-2.4.0.rsrc $(VTP_ROOT)/VTerrain_OSX/$(EXEC).app/Contents/Resources/$(EXEC).rsrc

clean :
	rm -rf $(OBJS) $(EXEC).core core ii_files

clobber :
	rm -rf $(OBJS) $(EXEC).core core ii_files $(EXEC) $(EXEC).pure
