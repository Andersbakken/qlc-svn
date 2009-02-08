include(../variables.pri)

TEMPLATE		= subdirs

# Common library
SUBDIRS			+= common

# Output plugins
!mac:SUBDIRS         	+= usbdmxout
!mac:unix:SUBDIRS	+= dmx4linuxout
!win32:SUBDIRS		+= ftdidmx
!mac:SUBDIRS		+= udmxout
!mac:SUBDIRS		+= midiout
unix:SUBDIRS		+= llaout

# Input plugins
!mac:SUBDIRS			+= midiinput
!mac:unix:SUBDIRS	+= hidinput
