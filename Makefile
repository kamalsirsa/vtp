#
# GNU Makefile:  VTerrain
#

VTP_ROOT = $(shell pwd)

all :
	cd TerrainSDK && $(MAKE)
	cd TerrainApps && $(MAKE)

install:
	cd TerrainSDK && $(MAKE)
	cd TerrainApps && $(MAKE)

clean :
	cd TerrainSDK && $(MAKE) clean
	cd TerrainApps && $(MAKE) clean

clobber :
	cd TerrainSDK && $(MAKE) clobber
	cd TerrainApps && $(MAKE) clobber
