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
	cd xmlhelper  ; $(MAKE) all
	cd vtlib/vtosg; $(MAKE) all
	cd vtui       ; $(MAKE) all
  
clean :
	cd vtdata     ; $(MAKE) clean
	cd xmlhelper  ; $(MAKE) clean
	cd vtlib/vtosg; $(MAKE) clean
	cd vtui       ; $(MAKE) clean

clobber :
	cd vtdata     ; $(MAKE) clobber
	cd xmlhelper  ; $(MAKE) clobber
	cd vtlib/vtosg; $(MAKE) clobber
	cd vtui       ; $(MAKE) clobber

