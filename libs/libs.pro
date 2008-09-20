TEMPLATE	= subdirs

SUBDIRS		+= common
SUBDIRS         += usbdmxout
SUBDIRS		+= midiinput

unix:SUBDIRS    += hidinput
unix:SUBDIRS	+= llaout
unix:SUBDIRS	+= dmx4linuxout
