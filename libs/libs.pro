TEMPLATE	= subdirs

SUBDIRS		+= common
SUBDIRS         += usbdmxout

unix:SUBDIRS    += hidinput
unix:SUBDIRS	+= llaout
unix:SUBDIRS	+= dmx4linuxout
