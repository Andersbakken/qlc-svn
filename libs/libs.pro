include(../variables.pri)

TEMPLATE		= subdirs

# Common library
SUBDIRS			+= common

# Output plugins
!mac:SUBDIRS         	+= usbdmxout
!mac:unix:SUBDIRS	+= dmx4linuxout
mac:SUBDIRS		+= ftdidmx

# These have not been tested but they should compile at least on unix
#!mac:unix:SUBDIRS	+= llaout
#!mac:SUBDIRS		+= udmxout

# Input plugins
SUBDIRS			+= midiinput
!mac:unix:SUBDIRS	+= hidinput
