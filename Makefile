#
# Makefile for the skinElchiHD Video Disk Recorder plugin
#
# See the main source file 'skinelchihd.c' for copyright information and
# how to reach the author.
#
# $Id$

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.

PLUGIN = skinelchihd

# External image lib to use: imagemagick or graphicsmagick
IMAGELIB ?= imagemagick

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'static const char \*VERSION *=' $(PLUGIN).c | awk '{ print $$6 }' | sed -e 's/[";]//g')

# Use package data if installed...otherwise assume we're under the VDR source directory:
PKG_CONFIG ?= pkg-config
PKGCFG = $(if $(VDRDIR),$(shell $(PKG_CONFIG) --variable=$(1) $(VDRDIR)/vdr.pc),$(shell $(PKG_CONFIG) --variable=$(1) vdr || $(PKG_CONFIG) --variable=$(1) ../../../vdr.pc))
LIBDIR = $(call PKGCFG,libdir)
LOCDIR = $(call PKGCFG,locdir)
PLGCFG = $(call PKGCFG,plgcfg)
#
TMPDIR ?= /tmp

### The compiler options:

export CFLAGS   = $(call PKGCFG,cflags)
export CXXFLAGS = $(call PKGCFG,cxxflags)
CXXFLAGS += -std=c++11
CXXFLAGS += -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable

### The version number of VDR's plugin API:

APIVERSION = $(call PKGCFG,apiversion)

### Allow user defined options to overwrite defaults:

-include $(PLGCFG)

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### The name of the shared object file:

SOFILE = libvdr-$(PLUGIN).so

### Includes and Defines (add further entries here):

ifeq ($(IMAGELIB), imagemagick)
$(warning Compiling with ImageMagick++)
INCLUDES += $(shell pkg-config --cflags Magick++)
LIBS += $(shell pkg-config --libs Magick++)
else ifeq ($(IMAGELIB), graphicsmagick)
$(warning Compiling with GraphicsMagick++)
INCLUDES += $(shell pkg-config --cflags GraphicsMagick++)
LIBS += $(shell pkg-config --libs GraphicsMagick++)
DEFINES += -DGRAPHICSMAGICK
else 
$(error ERROR: IMAGELIB must be either imagemagick or graphicsmagick)
endif

DEFINES += -DPLUGIN_NAME_I18N='"$(PLUGIN)"'

### The object files (add further files here):

OBJS = $(PLUGIN).o displaychannel.o displaymenu.o displayreplay.o displayvolume.o displaytracks.o displaymessage.o
OBJS += config.o background.o vdrstatus.o setup.o symbols.o image.o

### The main target:

all: $(SOFILE) i18n

### Implicit rules:

%.o: %.c
	@echo CC $@
	$(Q)$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) > $@

-include $(DEPFILE)

### Internationalization (I18N):

PODIR     = po
I18Npo    = $(wildcard $(PODIR)/*.po)
I18Nmo    = $(addsuffix .mo, $(foreach file, $(I18Npo), $(basename $(file))))
I18Nmsgs  = $(addprefix $(DESTDIR)$(LOCDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot   = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	@echo MO $@
	$(Q)msgfmt -c -o $@ $<

$(I18Npot): $(wildcard *.c)
	@echo GT $@
	$(Q)xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --package-name=vdr-$(PLUGIN) --package-version=$(VERSION) --msgid-bugs-address='<see README>' -o $@ `ls $^`

%.po: $(I18Npot)
	@echo PO $@
	$(Q)msgmerge -U --no-wrap --no-location --backup=none -q -N $@ $<
	@touch $@

$(I18Nmsgs): $(DESTDIR)$(LOCDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	@echo IN $@
	$(Q)install -D -m644 $< $@

.PHONY: i18n
i18n: $(I18Nmo) $(I18Npot)

install-i18n: $(I18Nmsgs)

### Targets:

$(SOFILE): $(OBJS)
	@echo LD $@
	$(Q)$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared $(OBJS) $(LIBS) -o $@

install-lib: $(SOFILE)
	$(Q)install -D $^ $(DESTDIR)$(LIBDIR)/$^.$(APIVERSION)

install: install-lib install-i18n

dist: $(I18Npo) clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar cjf $(PACKAGE).tar.bz2 -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tar.bz2

clean:
	@-rm -f $(PODIR)/*.mo $(PODIR)/*.pot
	@-rm -f $(OBJS) $(DEPFILE) *.so *.tar.bz2 *.tgz core* *~
