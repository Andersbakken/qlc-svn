TEMPLATE	= subdirs

# Common library
SUBDIRS		+= common

# Output plugins
SUBDIRS         += usbdmxout
unix:SUBDIRS	+= dmx4linuxout
unix:SUBDIRS	+= llaout

# Input plugins
SUBDIRS		+= midiinput
unix:SUBDIRS    += hidinput
