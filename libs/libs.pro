TEMPLATE	= subdirs

# Common library
SUBDIRS		+= common

# Output plugins
SUBDIRS         += usbdmxout
SUBDIRS		+= dmx4linuxout
SUBDIRS		+= llaout

# Input plugins
SUBDIRS		+= midiinput
unix:SUBDIRS    += hidinput
