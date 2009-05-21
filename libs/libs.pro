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

# LLA output plugin
exists($$LLA_GIT):exists($$PROTOBUF) {
	unix:SUBDIRS	+= llaout
}

# Input plugins
!mac:SUBDIRS		+= midiinput
!mac:unix:SUBDIRS	+= hidinput
SUBDIRS			+= ewinginput
