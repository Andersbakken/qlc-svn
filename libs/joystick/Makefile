#
# Q Light Controller
# Joystick shared library
# Makefile
#
# Copyright (C) 2000, 2001, 2002 Heikki Junnila
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# Version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details. The license is
# in the file "COPYING".
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

include ../../config.make

include uichfiles
include uicofiles
include mocuicofiles
include mocofiles
include objfiles

LIBJOY_SO = libjoy.so
LDFLAGS += -fPIC -shared
LIBS = -L$(QTDIR)/lib -L$(X11LIBS) -lXext -lX11 -L$(QTLIB) -ldl -lpthread

MOCOBJECTS = $(MOCO) \
	../common/moc_ledbar.o ../common/moc_plugin.o

OBJECTS = $(OBJS) \
	../common/ledbar.o ../common/plugin.o

all:	banner uichfiles uicofiles mocuicofiles mocofiles objfiles cppfiles \
	hfiles uifiles Makefile.dep $(UICH) $(UICO) $(MOCUICO) $(OBJECTS) \
	$(LIBJOY_SO)

uichfiles uicofiles mocuicofiles mocofiles objfiles cppfiles hfiles uifiles \
mocfiles:
	../../buildtools/create_objects.sh

banner:
	@echo "----------------- Building QLC 2 Joystick Library -----------------"

$(LIBJOY_SO): $(UICO) $(MOCUICO) $(OBJECTS) $(MOCOBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources 
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@

install:
	@if [ ! -d $(SYSTEMPATH) ]; then echo "Couldn't find an existing installation from" $(SYSTEMPATH); exit 1; fi
	@if [ ! -d $(SYSTEMPATH)/$(PLUGINPATH) ]; then mkdir $(SYSTEMPATH)/$(PLUGINPATH); fi
	@install -g root -o root -m 755 $(LIBJOY_SO) $(SYSTEMPATH)/$(PLUGINPATH)/$(LIBJOY_SO)
	@echo "Installed" $(LIBJOY_SO) "to" $(SYSTEMPATH)"/$(PLUGINPATH)"

clean: emacsclean
	rm -f *.o *.so *.bak uic_* moc_* Makefile.dep $(UICH)
	rm -f uichfiles uicofiles mocuicofiles mocofiles objfiles cppfiles \
	hfiles uifiles mocfiles

emacsclean:
	rm -f *~ #*

distclean: clean
	rm -rf CVS

Makefile.dep:
	rm -f Makefile.dep
	touch Makefile.dep
	makedepend -fMakefile.dep -Y *.h *.cpp &> /dev/null

include Makefile.dep

