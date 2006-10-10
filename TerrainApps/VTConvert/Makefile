#
# GNU Makefile:  TerrainApps/VTConvert
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)
EXEC   = VTConvert
include $(VTP_ROOT)/Make.defs

IFLAGS =  -I$(GDAL_INC)              \
	  -I$(TERRAIN_SDK_DIR)       

LDIRS  = \
	  $(SHAPE_LIB)                   \
	  $(GDAL_LIB)                    \
	  $(PNG_LIB)                     \
	  $(X_LIB)                       \
	  $(EXTRA_LDIRS)

SWITCHES := $(SWITCHES) -DUNIX -DUSE_OPENGL=1
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)
LDFLAGS  = $(LDIRFLAGS) \
	   $(EXTRA_LDFLAGS)

CXXSRC = $(filter-out pre.cpp, $(wildcard *.cpp))
OBJS   = $(CXXSRC:%.cpp=%.o)
LIBS   = -lgdal -lm

# Support of Mac OS X
ifeq ($(OS),Darwin)
OSX_APP = $(OSX_APPS)/$(EXEC).app/Contents/MacOS/$(EXEC)
LIBS   += -F$(FRAMEWORKS) \
	  -lobjc -framework Carbon \
	  -framework vtdata \
	  -framework xmlhelper -framework unzip \
	  -L$(FINK_LIB) \
	  -lz

all : $(OSX_APP)

else
# Support for all other Unix platforms
LIBS   +=  -lGL -lX11 $(EXTRA_LIBS)   \
	  -lxmlhelper -lvtdata -lunzip -lcurl -lbz2

LDIRS  += \
	  $(VTDATA_INST_DIR)             \
	  $(VTUI_INST_DIR)               \
          $(XMLHELPER_INST_DIR)          \
          $(UNZIP_INST_DIR)

all : $(EXEC)
endif

$(EXEC) : $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

$(OSX_APP) : $(OBJS) $(OSX_APPS)/$(EXEC).app
	$(LD) -o $(OSX_APP) $(OBJS) $(LDFLAGS) $(LIBS)

clean :
	rm -rf $(OBJS) $(EXEC).core core ii_files wx/ii_files

clobber :
	rm -rf $(OBJS) $(EXEC).core core ii_files wx/ii_files $(EXEC) $(EXEC).pure $(OSX_APPS)/$(EXEC).app
