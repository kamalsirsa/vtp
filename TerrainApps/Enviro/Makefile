#
# GNU Makefile:  TerrainApps/Enviro
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ../..; pwd)

include $(VTP_ROOT)/TerrainApps/Make.defs

IFLAGS = $(shell wx-config --cflags) \
	  -I$(TERRAIN_SDK_DIR)       \
	  -I$(OSG_INC)               \
	  -I$(GDAL_INC)              \
	  -I$(TERRAIN_APPS_DIR)/Enviro/wx/bitmap \
	  -I$(TERRAIN_APPS_DIR)/Enviro/wx/icons

LDIRS  = \
	  $(OSG_LIB)                     \
	  $(VTDATA_INST_DIR)             \
	  $(VTUI_INST_DIR)               \
	  $(VTOSG_INST_DIR)              \
	  $(XMLHELPER_INST_DIR)          \
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
EXEC   = wxEnviro
LIBS   = $(shell wx-config --libs) $(shell wx-config --gl-libs) \
	  -lgdal -lvtosg -lvtui -lvtdata -lxmlhelper $(MINI_LIB) \
	  -losgDB -losgText -losgUtil -losg -lnetcdf \
	  $(GLU_LIB) -lGL -lX11 $(EXTRA_LIBS) -lm 

all : $(EXEC)

$(EXEC) : $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

clean :
	rm -rf $(OBJS) $(EXEC).core core ii_files wx/ii_files

clobber :
	rm -rf $(OBJS) $(EXEC).core core ii_files wx/ii_files $(EXEC) $(EXEC).pure

#
# Internationalization steps (iso-8859-1)
#

I18N_SOURCES = \
	wx/app.cpp \
	wx/frame.cpp \
	wx/CameraDlg.cpp \
	wx/LayerDlg.cpp \
	wx/LocationDlg.cpp \
	wx/SceneGraphDlg.cpp \
	wx/StartupDlg.cpp \
	wx/TerrManDlg.cpp \
	wx/TParamsDlg.cpp \
	wx/Enviro_wdr.cpp \
	../../TerrainSDK/vtui/vtui_wdr.cpp \
	../../TerrainSDK/vtui/BuildingDlg.cpp

publish-de:
	cp de/Enviro.pox de/Enviro.po
	msgfmt de/Enviro.po -o de/Enviro.mo

update-de:
	xgettext -k_ -o Enviro.pot $(I18N_SOURCES)
	msgmerge --no-fuzzy-matching de/Enviro.po Enviro.pot -o de/Enviro.pox

publish-sv:
	cp sv/Enviro.pox sv/Enviro.po
	msgfmt sv/Enviro.po -o sv/Enviro.mo

update-sv:
	xgettext -k_ -o Enviro.pot $(I18N_SOURCES)
	msgmerge --no-fuzzy-matching sv/Enviro.po Enviro.pot -o sv/Enviro.pox

publish-pt:
	cp pt/Enviro.pox pt/Enviro.po
	msgfmt pt/Enviro.po -o pt/Enviro.mo

update-pt:
	xgettext -k_ -o Enviro.pot $(I18N_SOURCES)
	msgmerge --no-fuzzy-matching pt/Enviro.po Enviro.pot -o pt/Enviro.pox

publish-hu:
	cp hu/Enviro.pox hu/Enviro.po
	msgfmt hu/Enviro.po -o hu/Enviro.mo

update-hu:
	xgettext -k_ -o Enviro.pot $(I18N_SOURCES)
	msgmerge --no-fuzzy-matching hu/Enviro.po Enviro.pot -o hu/Enviro.pox

publish-uk:
	cp en_GB/Enviro.pox en_GB/Enviro.po
	msgfmt en_GB/Enviro.po -o en_GB/Enviro.mo

#
# Internationalization steps for Infra (iso-8859-1)
#

I18N_SOURCES_INFRA = \
	wx/Infra/InfraFrame.cpp \
	wx/Infra/Infra_wdr.cpp

publish-infra:
	cp sv/Infra.pox sv/Infra.po
	msgfmt sv/Infra.po -o sv/Infra.mo

update-infra:
	xgettext -k_ -o Infra.pot $(I18N_SOURCES_INFRA)
	msgmerge --no-fuzzy-matching sv/Infra.po Infra.pot -o sv/Infra.pox


