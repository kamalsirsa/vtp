#
# GNU Makefile:  TerrainApps/Enviro
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)
EXEC = Enviro
include $(VTP_ROOT)/Make.defs

IFLAGS = $(shell $(WX_DIR)/bin/wx-config --cflags) \
	  -I$(TERRAIN_SDK_DIR)       \
	  -I$(OSG_INC)               \
	  -I$(GDAL_INC)              \
	  -I$(TERRAIN_APPS_DIR)/Enviro/wx/bitmap \
	  -I$(TERRAIN_APPS_DIR)/Enviro/wx/icons \
	  -I$(X_INC)

LDIRS  = \
	  $(OSG_LIB)                     \
	  $(SHAPE_LIB)                   \
	  $(PNG_LIB)                     \
	  $(GDAL_LIB)                    \
	  $(NETCDF_LIB)                  \
	  $(X_LIB)                       \
	  $(EXTRA_LDIRS)

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
	 -lgdal -lGL $(MINI_LIB) \
	 -losg -losgDB -losgUtil -losgText \
	 -lm $(EXTRA_LIBS)

# Support of Mac OS X
ifeq ($(OS),Darwin)
OSX_APP = $(OSX_APPS)/$(EXEC).app/Contents/MacOS/$(EXEC)
LIBS   += -F$(FRAMEWORKS) \
	  -lobjc -framework Carbon \
	  -lOpenThreads \
	  -framework vtdata -framework vtui -framework vtosg \
	  -framework xmlhelper \
	  -L$(FINK_LIB) -lnetcdf \
	  -lX11 -L$(X11BASE)/lib \
	  -lz -lstdc++

all : $(OSX_APP)

else
# Support for all other Unix platforms
LIBS   +=  $(GLU_LIB) -lX11 \
	  -lxmlhelper -lvtdata -lvtui -lvtosg \
	  -lnetcdf -lmini -lquikgrid                

LDIR   +=\
	  $(VTDATA_INST_DIR)             \
	  $(VTUI_INST_DIR)               \
	  $(VTOSG_INST_DIR)              \
	  $(XMLHELPER_INST_DIR)

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
# Internationalization steps (iso-8859-1) for Enviro
#

I18N_SOURCES = \
	wx/app.cpp \
	wx/frame.cpp \
	wx/*Dlg.cpp \
	wx/Enviro_wdr.cpp \
	../../TerrainSDK/vtui/vtui_wdr.cpp \
	../../TerrainSDK/vtui/BuildingDlg.cpp

update-pot:
	xgettext --from-code=iso-8859-1 -k_ -o Enviro.pot $(I18N_SOURCES)

#----------
publish-af:
	cp af/Enviro.pox af/Enviro.po
	msgfmt af/Enviro.po -o af/Enviro.mo

update-af:
	msgmerge --no-fuzzy-matching af/Enviro.po Enviro.pot -o af/Enviro.pox

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
publish-uk:
	cp en_GB/Enviro.pox en_GB/Enviro.po
	msgfmt en_GB/Enviro.po -o en_GB/Enviro.mo

#
# Internationalization steps for Infra (iso-8859-1)
#

I18N_SOURCES_INFRA = \
	wx/Infra/InfraFrame.cpp \
	wx/Infra/InfraDlg.cpp \
	wx/Infra/Infra_wdr.cpp

publish-infra:
	cp sv/Infra.pox sv/Infra.po
	msgfmt sv/Infra.po -o sv/Infra.mo

update-infra:
	xgettext -k_ -o Infra.pot $(I18N_SOURCES_INFRA)
	msgmerge --no-fuzzy-matching sv/Infra.po Infra.pot -o sv/Infra.pox

