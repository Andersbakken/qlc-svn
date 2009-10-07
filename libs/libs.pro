include(../variables.pri)

TEMPLATE		= subdirs

# Common library
SUBDIRS			+= common

# Output plugins
SUBDIRS			+= ftdidmx
!mac:!win32:SUBDIRS	+= dmx4linuxout
!mac:SUBDIRS         	+= peperoniout
SUBDIRS			+= enttecdmxusbout
!mac:SUBDIRS		+= enttecdmxusbproout
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
