#
# GNU Makefile:  VTP
#

VTP_ROOT = $(shell pwd)

include $(VTP_ROOT)/Make.defs


all :
	@-cd TerrainSDK && $(MAKE)
	@-cd TerrainApps && $(MAKE)

install:
ifneq ($(OS),Darwin)
	@-cd TerrainSDK && $(MAKE) install
	@-cd TerrainApps && $(MAKE) install
endif

clean :
	@-cd TerrainSDK && $(MAKE) clean
	@-cd TerrainApps && $(MAKE) clean
ifeq ($(OS),Darwin)
	rm -rf $(OSX_APPS)/*.app
	(cd $(FRAMEWORKS)/ && rm -rf *.framework)
endif

clobber :
	@-cd TerrainSDK && $(MAKE) clobber
	@-cd TerrainApps && $(MAKE) clobber

createpatch:
	-svn diff > "patch.`date`.txt"

#
# To use this target, call it with the name the patchfile, like this:
#    make applypatch patchfile=patch123.txt
#
applypatch:
	patch -p0 < ${patchfile}

ifeq ($(OS),Darwin)
cleanosx:
	(cd $(FRAMEWORKS)/ && rm -rf *.framework)
	rm -rf $(OSX_APPS)/*.app
endif
