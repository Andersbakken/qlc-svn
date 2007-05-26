##############################################################################
#
# Q Light Controller
# configure.in
#
# Copyright (C) 2005 Heikki Junnila
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
#
##############################################################################

AC_INIT()
AC_CONFIG_AUX_DIR(config)

AM_INIT_AUTOMAKE(qlc, 2.6.1)
AM_CONFIG_HEADER(config.h)

##############################
# Constants
##############################

QT_VERSION=3.3

##############################################################################
# Fixed subdirectories' names
##############################################################################

PIXMAPS=pixmaps/$PACKAGE
AC_SUBST(PIXMAPS)

FIXTURES=fixtures
AC_SUBST(FIXTURES)

PLUGINS=$libdir/qlc
AC_SUBST(PLUGINS)

DOCUMENTS=doc
AC_SUBST(DOCUMENTS)

##############################################################################
# Check for some necessary tools
##############################################################################

AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
#AC_PROG_AWK
AC_PROG_LIBTOOL
#AC_PROG_RANLIB
AC_LANG_CPLUSPLUS

##############################################################################
# Checks for standard C headers
##############################################################################

AC_HEADER_STDC
AC_CHECK_HEADERS([X11/Xlib.h])

##############################################################################
# Check for GNU make
##############################################################################
AC_CHECK_PROG(MAKE_PROG, make, yes, no)
if [ test $MAKE_PROG = no ]; then
	AC_MSG_ERROR(QLC needs make)
fi

##############################################################################
# Check for QT libraries & headers (as well as pkg-config)
##############################################################################
PKG_CHECK_MODULES(QT, [qt-mt >= 3.3.0])
AC_SUBST(QT_CFLAGS)
AC_SUBST(QT_LIBS)

##############################################################################
# Check for QT's User Interface Compiler (UIC)
##############################################################################
AC_CHECK_PROG(UIC, uic, yes, no)
if [ test $UIC = yes ]; then
	UIC=`which uic`
	AC_SUBST(UIC)
else
	AC_MSG_ERROR(QLC needs QT >= $QT_VERSION)
fi

##############################################################################
# Check for QT's Meta Object Compiler (MOC)
##############################################################################
AC_CHECK_PROG(MOC, moc, yes, no)
if [ test $MOC = yes ]; then
	MOC=`which moc`
	AC_SUBST(MOC)
else
	AC_MSG_ERROR(QLC needs QT >= $QT_VERSION)
fi

##############################################################################
# DMX4Linux
##############################################################################
AC_ARG_ENABLE(dmx4linux,
	AS_HELP_STRING([--enable-dmx4linux],
		[enable DMX4Linux plugin support (default = yes)]),
	[
		if [ test $enableval = no ]; then
			dmx4linux=no
		else
			dmx4linux=yes
		fi
	],[
		dmx4linux=yes
	]
)
AM_CONDITIONAL(USE_DMX4LINUX, test $dmx4linux = yes)

##############################################################################
# Peperoni USB2DMX
##############################################################################
AC_ARG_ENABLE(usbdmxout,
	AS_HELP_STRING([--enable-usbdmx],
		[enable Peperoni USBDMX plugin support (default = yes)]),
	[
		if [ test $enableval = no ]; then
			usbdmxout=no
		else
			usbdmxout=yes
		fi
	],[
		usbdmxout=yes
	]
)
AM_CONDITIONAL(USE_USBDMXOUT, test $usbdmxout = yes)

##############################################################################
# MIDI out
##############################################################################
AC_ARG_ENABLE(midiout,
	AS_HELP_STRING([--enable-midiout],
		[enable MIDI plugin support (default = no)]),
	[
		if [ test $enableval = yes ]; then
			midiout=yes
		else
			midiout=no
		fi
	],[
		midiout=no
	]
)
AM_CONDITIONAL(USE_MIDIOUT, test $midiout = yes)

##############################################################################
# Sound to Light
##############################################################################
AC_ARG_ENABLE(sound2light,
	AS_HELP_STRING([--enable-sound2light],
		[enable Sound 2 light plugin support (default = no)]),
	[
		if [ test $enableval = yes ]; then
			sound2light=yes
		else
			sound2light=no
		fi
	],[
		sound2light=no
	]
)
AM_CONDITIONAL(USE_SOUND2LIGHT, test $sound2light = yes)

##############################################################################
# MIDI in/out
##############################################################################
AC_ARG_ENABLE(midiinout,
	AS_HELP_STRING([--enable-midiinout],
		[enable MIDI in/out plugin support (default = no)]),
	[
		if [ test $enableval = yes ]; then
			midiinout=yes
		else
			midiinout=no
		fi
	],[
		midiinout=no
	]
)
AM_CONDITIONAL(USE_MIDIINOUT, test $midiinout = yes)

##############################################################################
# LLA
##############################################################################
AC_ARG_ENABLE(lla,
	AS_HELP_STRING([--enable-lla],
		[enable LLA plugin support (default = no)]),
	[
		if [ test $enableval = yes ]; then
			lla=yes
			PKG_CHECK_MODULES(LIBLLA, [liblla >= 0.2.0])
		else
			lla=no
		fi
	],[
		lla=no
	]
)
AM_CONDITIONAL(USE_LLA, test $lla = yes)

AC_SUBST(LIBLLA_CFLAGS)
AC_SUBST(LIBLLA_LDFLAGS)

##############################################################################
# Generated Makefiles
##############################################################################

AC_OUTPUT([
	Makefile
	libs/Makefile
	libs/common/Makefile
	libs/dmx4linuxout/Makefile
	libs/usbdmxout/Makefile
	libs/midiout/Makefile
	libs/midi-inout/Makefile
	libs/soundtolight/Makefile
	libs/lla/Makefile
	dce/Makefile
	main/Makefile
	devices/Makefile
	gfx/Makefile
	docs/Makefile
	etc/Makefile
])
