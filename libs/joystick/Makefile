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

LIBJOY_SO = libjoy.so
LDFLAGS += -fPIC -shared
LIBS = -L$(QTDIR)/lib -L$(X11LIBS) -lXext -lX11 -L$(QTLIB) -ldl -lpthread

UICH =  uic_selectjoystick.h

UICO =  uic_selectjoystick.o \
	moc_uic_selectjoystick.o

MOCS =  ../common/moc_ledbar.o ../common/moc_plugin.o \
	moc_joystick.o moc_joystickplugin.o moc_selectjoystick.o \
	moc_joysticksettings.o

OBJS =	$(UICO) \
	$(MOCS) \
	../common/ledbar.o ../common/plugin.o \
	joystick.o joystickplugin.o selectjoystick.o joysticksettings.o

all:	banner dep $(UICH) $(UICO) $(LIBJOY_SO)

banner:
	@echo "----------------- Building QLC 2 Joystick Library -----------------"

$(LIBJOY_SO): $(OBJS)
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

emacsclean:
	rm -f *~ #*

distclean: clean
	rm -rf CVS

dep:
	rm -f Makefile.dep
	touch Makefile.dep
	makedepend -fMakefile.dep -Y *.h *.cpp &> /dev/null

-include Makefile.dep
