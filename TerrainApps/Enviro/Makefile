#
# GNU Makefile:  TerrainApps/Enviro
#

VTP_ROOT = $(shell cd ../..; pwd)
EXEC = Enviro
include $(VTP_ROOT)/Make.defs

IFLAGS = $(shell $(WX_DIR)/bin/wx-config --cflags) \
	 -I$(TERRAIN_SDK_DIR)       \
	 -I$(OSG_INC)               \
	 -I$(GDAL_INC)              \
	 -I$(MINI_INC)              \
	 -I$(SQUISH_INC)            \
	 -I$(TERRAIN_APPS_DIR)/Enviro/wx/bitmap \
	 -I$(TERRAIN_APPS_DIR)/Enviro/wx/icons
ifneq ($(OS),Darwin)
IFLAGS += -I$(X_INC)
endif

LDIRS  = $(VTDATA_INST_DIR)         \
	 $(VTOSG_INST_DIR)          \
	 $(VTUI_INST_DIR)           \
	 $(XMLHELPER_INST_DIR)      \
	 $(UNZIP_INST_DIR)          \
	 $(OSG_LIB)                 \
	 $(SHAPE_LIB)               \
	 $(PNG_LIB)                 \
	 $(GDAL_LIB)                \
	 $(EXTRA_LDIRS)
ifneq ($(OS),Darwin)
LDIRS += $(X_LIB)
endif

SWITCHES := $(SWITCHES) -DUNIX -DVTLIB_OSG=1
CFLAGS   = $(COPT) $(SWITCHES) $(IFLAGS)
CXXFLAGS = $(COPT) $(SWITCHES) $(IFLAGS)
LDFLAGS  = $(LDIRFLAGS) \
	   $(EXTRA_LDFLAGS)

CXXSRC = $(wildcard *.cpp) \
	 $(filter-out wx/StartupDlg0.cpp, $(wildcard wx/*.cpp))
OBJS   = $(CXXSRC:%.cpp=%.o)
LIBS   = $(shell $(WX_DIR)/bin/wx-config --libs) \
	 $(shell $(WX_DIR)/bin/wx-config --gl-libs) \
	 -lgdal \
	 -lm

ifeq ($(OS),Darwin)
# Support for Mac OS X
OSX_APP = $(OSX_APPS)/$(EXEC).app/Contents/MacOS/$(EXEC)
LIBS   += -F$(FRAMEWORKS) \
	  -lobjc \
	  $(call _VLF,vtdata) $(call _VLF,vtui) $(call _VLF,vtosg) \
	  $(call _VLF,xmlhelper) $(call _VLF,unzip) \
	  $(OSG_OPTS) \
	  -framework OpenGL -framework AGL \
	  -L$(FINK_LIB) \
	  -lz -lstdc++ \
	  -Wl,-executable_path -Wl,$(OSX_APPS)/$(EXEC).app/Contents/MacOS \
	  -bind_at_load
else
# Support for all other Unix platforms
LIBS   +=  -lxmlhelper -lunzip -lvtdata -lvtui -lvtosg -lminidata $(MINI_LIB) -lGL $(GLU_LIB) -lX11 $(EXTRA_LIBS) \
	 -losg -losgDB -losgUtil -losgText -losgParticle -losgShadow -lOpenThreads
endif

ifeq ($(SUPPORT_HTTP),1)
	LIBS += -lcurl
endif
ifeq ($(SUPPORT_BZIP2),1)
	LIBS += -lbz2
endif

LDIRS   +=\
	  $(MINIDATA_INST_DIR)           \
	  $(VTDATA_INST_DIR)             \
	  $(VTUI_INST_DIR)               \
	  $(VTOSG_INST_DIR)              \
	  $(XMLHELPER_INST_DIR)


ifeq ($(OS),Darwin)
all : $(OSX_APP)
else
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
# Internationalization steps for Enviro
#

I18N_SOURCES = \
	wx/*.cpp \
	Enviro.cpp \
	../../TerrainSDK/vtui/*.cpp

update-pot:
	xgettext --from-code=iso-8859-1 -k_ -o Enviro.pot $(I18N_SOURCES)

#----------
publish-en:
	cp en/Enviro.pox en/Enviro.po
	msgfmt en/Enviro.po -o en/Enviro.mo

#----------
publish-engb:
	cp en_GB/Enviro.pox en_GB/Enviro.po
	msgfmt en_GB/Enviro.po -o en_GB/Enviro.mo

#----------
publish-af:
	cp af/Enviro.pox af/Enviro.po
	msgfmt af/Enviro.po -o af/Enviro.mo

update-af:
	msgmerge --no-fuzzy-matching af/Enviro.po Enviro.pot -o af/Enviro.pox

#----------
publish-ar:
	cp ar/Enviro.pox ar/Enviro.po
	msgfmt ar/Enviro.po -o ar/Enviro.mo

update-ar:
	msgmerge --no-fuzzy-matching ar/Enviro.po Enviro.pot -o ar/Enviro.pox

#----------
publish-zh:
	cp zh/Enviro.pox zh/Enviro.po
	msgfmt zh/Enviro.po -o zh/Enviro.mo

update-zh:
	msgmerge --no-fuzzy-matching zh/Enviro.po Enviro.pot -o zh/Enviro.pox

#----------
publish-de:
	cp de/Enviro.pox de/Enviro.po
	msgfmt de/Enviro.po -o de/Enviro.mo

update-de:
	msgmerge --no-fuzzy-matching de/Enviro.po Enviro.pot -o de/Enviro.pox

#----------
publish-es:
	cp es/Enviro.pox es/Enviro.po
	msgfmt es/Enviro.po -o es/Enviro.mo

update-es:
	msgmerge --no-fuzzy-matching es/Enviro.po Enviro.pot -o es/Enviro.pox

#----------
publish-fr:
	cp fr/Enviro.pox fr/Enviro.po
	msgfmt fr/Enviro.po -o fr/Enviro.mo

update-fr:
	msgmerge --no-fuzzy-matching fr/Enviro.po Enviro.pot -o fr/Enviro.pox

#----------
publish-hu:
	cp hu/Enviro.pox hu/Enviro.po
	msgfmt hu/Enviro.po -o hu/Enviro.mo

update-hu:
	msgmerge --no-fuzzy-matching hu/Enviro.po Enviro.pot -o hu/Enviro.pox

#----------
publish-it:
	cp it/Enviro.pox it/Enviro.po
	msgfmt it/Enviro.po -o it/Enviro.mo

update-it:
	msgmerge --no-fuzzy-matching it/Enviro.po Enviro.pot -o it/Enviro.pox

#----------
publish-pt:
	cp pt/Enviro.pox pt/Enviro.po
	msgfmt pt/Enviro.po -o pt/Enviro.mo

update-pt:
	msgmerge --no-fuzzy-matching pt/Enviro.po Enviro.pot -o pt/Enviro.pox

#----------
publish-ro:
	cp ro/Enviro.pox ro/Enviro.po
	msgfmt ro/Enviro.po -o ro/Enviro.mo

update-ro:
	msgmerge --no-fuzzy-matching ro/Enviro.po Enviro.pot -o ro/Enviro.pox

#----------
publish-sv:
	cp sv/Enviro.pox sv/Enviro.po
	msgfmt sv/Enviro.po -o sv/Enviro.mo

update-sv:
	msgmerge --no-fuzzy-matching sv/Enviro.po Enviro.pot -o sv/Enviro.pox

#----------
publish-tr:
	cp tr/Enviro.pox tr/Enviro.po
	msgfmt tr/Enviro.po -o tr/Enviro.mo

update-tr:
	msgmerge --no-fuzzy-matching tr/Enviro.po Enviro.pot -o tr/Enviro.pox

#----------
publish-uk:
	cp en_GB/Enviro.pox en_GB/Enviro.po
	msgfmt en_GB/Enviro.po -o en_GB/Enviro.mo
