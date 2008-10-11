TEMPLATE	= subdirs

# Common library
SUBDIRS		+= common

# Output plugins
SUBDIRS         += usbdmxout
unix:SUBDIRS	+= dmx4linuxout
!mac:unix:SUBDIRS	+= llaout

# Input plugins
SUBDIRS		+= midiinput
!mac:unix:SUBDIRS    += hidinput
