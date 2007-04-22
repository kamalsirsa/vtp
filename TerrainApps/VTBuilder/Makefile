#
# GNU Makefile:  TerrainApps/VTBuilder
#

VTP_ROOT = $(shell cd ../..; pwd)
EXEC   = VTBuilder
include $(VTP_ROOT)/Make.defs

IFLAGS = $(shell $(WX_DIR)/bin/wx-config --cflags) \
	  -I$(GDAL_INC)              \
	  -I$(TERRAIN_SDK_DIR)       \
	  -I$(TERRAIN_APPS_DIR)/VTBuilder/bitmaps \
	  -I$(TERRAIN_APPS_DIR)/VTBuilder/icons

LDIRS  = \
	  $(SHAPE_LIB)                   \
	  $(GDAL_LIB)                    \
	  $(PNG_LIB)                     \
	  $(X_LIB)                       \
	  $(EXTRA_LDIRS)

ifeq ($(OS),Darwin)
SWITCHES := $(SWITCHES) -DUSE_OPENGL=0
else
SWITCHES := $(SWITCHES) -DUSE_OPENGL=1
endif

LDFLAGS  = $(LDIRFLAGS) \
	   $(EXTRA_LDFLAGS)

CXXSRC = $(filter-out pre.cpp, $(wildcard *.cpp))
OBJS   = $(CXXSRC:%.cpp=%.o)
LIBS   = $(shell $(WX_DIR)/bin/wx-config --libs) \
	 $(shell $(WX_DIR)/bin/wx-config --gl-libs) \
	-lgdal -lm

# Support of Mac OS X
ifeq ($(OS),Darwin)
OSX_APP = $(OSX_APPS)/$(EXEC).app/Contents/MacOS/$(EXEC)
LIBS   += -F$(FRAMEWORKS) \
	  -lobjc -framework Carbon \
	  -framework vtdata -framework vtui \
	  -framework xmlhelper -framework unzip \
	  -L$(FINK_LIB) -lpng \
	  -lz \
  	  -Wl,-executable_path -Wl,$(OSX_APPS)/$(EXEC).app/Contents/MacOS


all : $(OSX_APP)

else
# Support for all other Unix platforms
LIBS   +=  -lGL -lX11 $(EXTRA_LIBS)   \
	  -lxmlhelper -lvtdata -lvtui -lunzip -lcurl -lbz2

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

#
# Internationalization steps for VTBuilder
#

I18N_SOURCES = \
	$(CXXSRC) \
	../../TerrainSDK/vtui/vtui_wdr.cpp \
	../../TerrainSDK/vtui/*Dlg.cpp

update-pot:
	xgettext --from-code=iso-8859-1 -k_ -o VTBuilder.pot $(I18N_SOURCES)

publish-ar:
	cp ar/VTBuilder.pox ar/VTBuilder.po
	msgfmt ar/VTBuilder.po -o ar/VTBuilder.mo

update-ar:
	msgmerge --no-fuzzy-matching ar/VTBuilder.po VTBuilder.pot -o ar/VTBuilder.pox

publish-de:
	cp de/VTBuilder.pox de/VTBuilder.po
	msgfmt de/VTBuilder.po -o de/VTBuilder.mo

update-de:
	msgmerge --no-fuzzy-matching de/VTBuilder.po VTBuilder.pot -o de/VTBuilder.pox

publish-fr:
	cp fr/VTBuilder.pox fr/VTBuilder.po
	msgfmt fr/VTBuilder.po -o fr/VTBuilder.mo

update-fr:
	msgmerge --no-fuzzy-matching fr/VTBuilder.po VTBuilder.pot -o fr/VTBuilder.pox

publish-it:
	cp it/VTBuilder.pox it/VTBuilder.po
	msgfmt it/VTBuilder.po -o it/VTBuilder.mo

update-it:
	msgmerge --no-fuzzy-matching it/VTBuilder.po VTBuilder.pot -o it/VTBuilder.pox

publish-ro:
	cp ro/VTBuilder.pox ro/VTBuilder.po
	msgfmt ro/VTBuilder.po -o ro/VTBuilder.mo

update-ro:
	msgmerge --no-fuzzy-matching ro/VTBuilder.po VTBuilder.pot -o ro/VTBuilder.pox

publish-zh:
	cp zh/VTBuilder.pox zh/VTBuilder.po
	msgfmt zh/VTBuilder.po -o zh/VTBuilder.mo

update-zh:
	msgmerge --no-fuzzy-matching zh/VTBuilder.po VTBuilder.pot -o zh/VTBuilder.pox

