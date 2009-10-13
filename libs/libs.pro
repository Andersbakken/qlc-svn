include(../variables.pri)

TEMPLATE		= subdirs

# Common library
SUBDIRS			+= common

# Helper libraries for macx plugins
macx:SUBDIRS		+= macx

# Output plugins
SUBDIRS			+= enttecdmxusbout
SUBDIRS			+= enttecdmxusbproout
SUBDIRS         	+= peperoniout
SUBDIRS			+= udmxout
!mac:!win32:SUBDIRS	+= dmx4linuxout
!mac:SUBDIRS		+= midiout

# OLA output plugin
exists($$OLA_GIT) {
	unix:SUBDIRS	+= olaout
}

# Input plugins
SUBDIRS			+= ewinginput
!mac:SUBDIRS		+= midiinput
!mac:!win32:SUBDIRS	+= hidinput
