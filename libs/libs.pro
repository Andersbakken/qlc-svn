TEMPLATE	= subdirs

# Common library
SUBDIRS		+= common

# Output plugins
!mac:SUBDIRS         	+= usbdmxout
!mac:unix:SUBDIRS	+= dmx4linuxout
!mac:unix:SUBDIRS	+= llaout
!win32:SUBDIRS		+= serialdmx

# Input plugins
SUBDIRS		+= midiinput
!mac:unix:SUBDIRS    += hidinput
