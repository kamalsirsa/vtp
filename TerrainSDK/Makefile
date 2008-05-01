#
# GNU Makefile:  TerrainSDK
#

#------------------------------------------------------------------------------
# You shouldn't need to tweak below this line
#------------------------------------------------------------------------------

VTP_ROOT = $(shell cd ..; pwd)

include $(VTP_ROOT)/Make.defs

all :
ifeq ($(OS),Darwin)
	mkdir -p $(FRAMEWORKS)
endif
	cd unzip      && $(MAKE) all
	cd xmlhelper  && $(MAKE) all
	cd vtdata     && $(MAKE) all
	cd vtlib/vtosg&& $(MAKE) all
	cd vtui       && $(MAKE) all

clean :
	cd unzip      && $(MAKE) clean
	cd xmlhelper  && $(MAKE) clean
	cd vtdata     && $(MAKE) clean
	cd vtlib/vtosg&& $(MAKE) clean
	cd vtui       && $(MAKE) clean

clobber :
	cd unzip      && $(MAKE) clobber
	cd xmlhelper  && $(MAKE) clobber
	cd vtdata     && $(MAKE) clobber
	cd vtlib/vtosg&& $(MAKE) clobber
	cd vtui       && $(MAKE) clobber

install :
	cd unzip      && $(MAKE) install
	cd xmlhelper  && $(MAKE) install
	cd vtdata     && $(MAKE) install
	cd vtlib/vtosg&& $(MAKE) install
	cd vtui       && $(MAKE) install
