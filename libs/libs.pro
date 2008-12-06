include(../variables.pri)

TEMPLATE		= subdirs

# Common library
SUBDIRS			+= common

# Output plugins
!mac:SUBDIRS         	+= usbdmxout
!mac:unix:SUBDIRS	+= dmx4linuxout
!win32:SUBDIRS		+= ftdidmx
!mac:SUBDIRS		+= udmxout
unix:SUBDIRS		+= midiout
unix:SUBDIRS		+= llaout

# Input plugins
SUBDIRS			+= midiinput
!mac:unix:SUBDIRS	+= hidinput
