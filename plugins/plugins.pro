TEMPLATE		= subdirs

# Output plugins
SUBDIRS			+= enttecdmxusbout
SUBDIRS         	+= peperoniout
SUBDIRS			+= udmxout
SUBDIRS			+= midiout
!macx:!win32:SUBDIRS	+= dmx4linuxout
exists($$OLA_GIT) {
	unix:SUBDIRS	+= olaout
}

# Input plugins
SUBDIRS			+= ewinginput
SUBDIRS			+= midiinput
!mac:!win32:SUBDIRS	+= hidinput
