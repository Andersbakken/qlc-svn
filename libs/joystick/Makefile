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

include config.make

OBJS =	libjoy.o

all: banner $(LIBJOY_SO)

banner:
	@echo "----------------- Building QLC 2.0 Joystick Library -----------------"

$(LIBJOY_SO): $(OBJS)
	$(CXX) -shared -lpthread -fPIC -o $@ $^

test:
	gcc test.c -lpthread -o test -ldl

# obsolete
libjoy:
	gcc -fPIC -c libjoy.c
	gcc -shared -fPIC -lpthread -o $(LIBJOY_SO) libjoy.o

dep:
	makedepend -fMakefile.dep -Y *.h *.c &> /dev/null

install:
	@if [ ! -d $(INSTALLDIR) ]; then echo "Couldn't find an existing installation from" $(INSTALLDIR); exit 1; fi
	@if [ ! -d $(INSTALLDIR)/libs ]; then mkdir $(INSTALLDIR)/libs; fi
	@install -g root -o root -m 755 $(LIBJOY_SO) $(INSTALLDIR)/libs/$(LIBJOY_SO)
	@echo "Installed" $(LIBJOY_SO) "to" $(INSTALLDIR)"/libs"

clean: emacsclean
	rm -f *.o *.so test *.bak

emacsclean:
	rm -f *~

-include Makefile.dep
