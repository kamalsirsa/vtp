#
# GNU Makefile:  TerrainApps
#

VTP_ROOT = $(shell cd ..; pwd)

include $(VTP_ROOT)/Make.defs

APPS = glutSimple wxSimple VTBuilder Enviro CManager
# prSimple is currently broken on OS X; per Ben. Removing it from the default targets.
all:
	@-for app in $(APPS); do            \
	  cd $(VTP_ROOT)/TerrainApps/$$app; \
	  $(MAKE) all || exit 1;            \
	  done

clean:
	@-for app in $(APPS); do            \
	  cd $(VTP_ROOT)/TerrainApps/$$app; \
	  $(MAKE) clean || exit 1;          \
	  done

clobber:
	@-for app in $(APPS); do            \
	  cd $(VTP_ROOT)/TerrainApps/$$app; \
	  $(MAKE) clobber || exit 1;        \
	  done

install:
	@-# currently nothing to do

#
# The following allows you to give the command "make VTBuilder" from the
#  TerrainApps folder.  To avoid the actual directories (such as 'VTBuilder')
#  being found as the target, the followin ".PHONY" target tricks the
#  makefile.
# This hack is by Nino Walker.
#
.PHONY: VTBuilder Enviro CManager

VTBuilder: 
	pwd
	cd $(VTP_ROOT)/TerrainApps/VTBuilder && \
	  $(MAKE) all

Enviro:
	cd $(VTP_ROOT)/TerrainApps/Enviro && \
	  $(MAKE) all

CManager:
	cd $(VTP_ROOT)/TerrainApps/CManager && \
	  $(MAKE) all
