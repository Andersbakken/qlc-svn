include(../variables.pri)

TEMPLATE		= subdirs
CONFIG			+= ordered

# Common library
SUBDIRS			+= common

# Output plugins
SUBDIRS			+= enttecdmxusbout
SUBDIRS         	+= peperoniout
SUBDIRS			+= udmxout
SUBDIRS			+= midiout
!macx:!win32:SUBDIRS	+= dmx4linuxout

# OLA output plugin
exists($$OLA_GIT) {
	unix:SUBDIRS	+= olaout
}

# Input plugins
SUBDIRS			+= ewinginput
SUBDIRS			+= midiinput
!mac:!win32:SUBDIRS	+= hidinput

# Unit tests
SUBDIRS			+= common/test
SUBDIRS			+= ewinginput/test
