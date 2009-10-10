include(../variables.pri)

TEMPLATE		= subdirs

# Common library
SUBDIRS			+= common

# Output plugins
SUBDIRS			+= enttecdmxusbout
SUBDIRS			+= enttecdmxusbproout
!mac:!win32:SUBDIRS	+= dmx4linuxout
!mac:SUBDIRS         	+= peperoniout
!mac:SUBDIRS		+= udmxout
!mac:SUBDIRS		+= midiout

# OLA output plugin
exists($$OLA_GIT) {
	unix:SUBDIRS	+= olaout
}

# Input plugins
SUBDIRS			+= ewinginput
!mac:SUBDIRS		+= midiinput
!mac:!win32:SUBDIRS	+= hidinput
