#
# GNU Makefile:  TerrainSDK
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ..; pwd)

include $(VTP_ROOT)/TerrainSDK/Make.defs

all :
	cd vtdata     ; $(MAKE) all
	cd vtlib/vtosg; $(MAKE) all

clean :
	cd vtdata     ; $(MAKE) clean
	cd vtlib/vtosg; $(MAKE) clean

clobber :
	cd vtdata     ; $(MAKE) clobber
	cd vtlib/vtosg; $(MAKE) clobber

