#
# GNU Makefile:  TerrainApps
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ..; pwd)

include $(VTP_ROOT)/TerrainApps/Make.defs

APPS = glutSimple wxSimple prSimple VTBuilder Enviro

all :
	for app in $(APPS); do              \
	  cd $(VTP_ROOT)/TerrainApps/$$app; \
	  $(MAKE) all || exit 1;            \
	  done

clean :
	for app in $(APPS); do              \
	  cd $(VTP_ROOT)/TerrainApps/$$app; \
	  $(MAKE) clean || exit 1;          \
	  done

clobber :
	for app in $(APPS); do              \
	  cd $(VTP_ROOT)/TerrainApps/$$app; \
	  $(MAKE) clobber || exit 1;        \
	  done

