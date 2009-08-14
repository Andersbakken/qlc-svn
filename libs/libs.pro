include(../variables.pri)

TEMPLATE		= subdirs

# Common library
SUBDIRS			+= common

# Output plugins
!mac:SUBDIRS         	+= usbdmxout
!mac:unix:SUBDIRS	+= dmx4linuxout
SUBDIRS			+= ftdidmx
!mac:SUBDIRS		+= udmxout
!mac:SUBDIRS		+= midiout

# OLA output plugin
exists($$OLA_GIT) {
	unix:SUBDIRS	+= olaout
}

# Input plugins
!mac:SUBDIRS		+= midiinput
!mac:unix:SUBDIRS	+= hidinput
SUBDIRS			+= ewinginput
