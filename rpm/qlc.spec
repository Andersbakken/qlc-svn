#############################################################################
# QLC Master package
#############################################################################

%define version 3.0.4

Summary: Q Light Controller - The free DMX lighting console
License: GPLv2
Group: Other
Name: qlc
Prefix: /usr
Provides: qlc
Requires: qt
Release: 1
Source: qlc-%{version}.tar.gz
URL: http://www.sf.net/projects/qlc
Version: %{version}
Buildroot: /tmp/qlcrpm
%description
Q Light Controller - The free Linux DMX lighting desk. Includes also fixture definitions and input profiles.

%prep
%setup -q

%build
qmake-qt4
make

%install
rm -rf $RPM_BUILD_ROOT
INSTALL_ROOT=$RPM_BUILD_ROOT make install

#%clean
#rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/bin/qlc
/usr/share/applications/qlc.desktop
/usr/share/pixmaps/qlc.png
/usr/bin/qlc-fixtureeditor
/usr/share/applications/qlc-fixtureeditor.desktop
/usr/share/pixmaps/qlc-fixtureeditor.png
/usr/share/qlc/fixtures/*
/usr/share/qlc/inputprofiles/*
%doc /usr/share/qlc/documents/*

#############################################################################
# HID input plugin sub-package
#############################################################################

%package input-hid
Requires: qlc
Group: Other
Summary: Input support for HID (Human Interface Devices)
%description input-hid
Input plugin for Q Light Controller that enables input data from
HID-compliant devices (for example joysticks, mice and keyboards).

%files input-hid
%defattr(-,root,root) 
/usr/lib/qlc/input/libhidinput.so

#############################################################################
# Enttec Wing input plugin sub-package
#############################################################################

%package input-enttec-wing
Requires: qlc
Group: Other
Summary: Input support for Enttec wing hardware
%description input-enttec-wing
Input plugin for Q Light Controller that enables input data from
ENTTEC Shortcut, Playback and Program wings.

%files input-enttec-wing
%defattr(-,root,root) 
/usr/lib/qlc/input/libewinginput.so

#############################################################################
# MIDI input plugin sub-package
#############################################################################

%package input-midi
Requires: qlc alsa-lib
Group: Other
Summary: Input support for MIDI devices
%description input-midi
Input plugin for Q Light Controller that enables input data from
MIDI controller devices (such as Behringer BCF2000).

%files input-midi
%defattr(-,root,root) 
/usr/lib/qlc/input/libmidiinput.so

#############################################################################
# DMX4Linux output plugin sub-package
#############################################################################

%package output-dmx4linux
Requires: qlc
Group: Other
Summary: DMX output support for DMX4Linux driver suite
%description output-dmx4linux
Output plugin for Q Light Controller that enables DMX output support for
devices supported by the DMX4Linux driver suite (http://llg.cubic.org/)

%files output-dmx4linux
%defattr(-,root,root) 
/usr/lib/qlc/output/libdmx4linuxout.so

#############################################################################
# ENTTEC DMX USB Open/Pro output plugin sub-package
#############################################################################

%package output-enttec-dmxusb
Requires: qlc
Group: Other
Summary: DMX output support for Enttec DMX USB dongles (D2XX interface)
%description output-enttec-dmxusb
Output plugin for Q Light Controller that enables DMX output support for
ENTTEC Open DMX USB and DMX USB Pro dongles, using the legacy D2XX interface.

%files output-enttec-dmxusb
%defattr(-,root,root) 
/usr/lib/qlc/output/libenttecdmxusbout.so
/etc/udev/rules.d/z65-enttec-dmxusb.rules

#############################################################################
# ENTTEC DMX USB Pro output plugin sub-package
#############################################################################

%package output-enttec-dmxusbpro
Requires: qlc
Group: Other
Summary: DMX output support for Enttec DMX USB Pro dongle (new VCP interface)
%description output-enttec-dmxusbpro
Output plugin for Q Light Controller that enables DMX output support for the
ENTTEC DMX USB Pro dongle, using the new VCP (Virtual Communications Port)
interface.

%files output-enttec-dmxusbpro
%defattr(-,root,root) 
/usr/lib/qlc/output/libenttecdmxusbproout.so

#############################################################################
# Anyma uDMX output plugin sub-package
#############################################################################

%package output-udmx
Requires: qlc libusb
Group: Other
Summary: DMX output support for Anyma uDMX devices
%description output-udmx
Output plugin for Q Light Controller that enables DMX output support for
Peperoni USB devices (Rodin, USBDMX21, XSwitch).

%files output-udmx
%defattr(-,root,root)
/usr/lib/qlc/output/libudmxout.so
/etc/udev/rules.d/z65-anyma-udmx.rules

#############################################################################
# Peperoni output plugin sub-package
#############################################################################

%package output-peperoni
Requires: qlc libusb
Group: Other
Summary: DMX output support for Peperoni USB devices (Rodin, USBDMX21, XSwitch)
%description output-peperoni
Output plugin for Q Light Controller that enables DMX output support for
Peperoni USB devices (Rodin, USBDMX21, XSwitch).

%files output-peperoni
%defattr(-,root,root) 
/usr/lib/qlc/output/libpeperoniout.so
/etc/udev/rules.d/z65-peperoni.rules

#############################################################################
# MIDI output plugin sub-package
#############################################################################

%package output-midi
Requires: qlc alsa-lib
Group: Other
Summary: DMX output support for MIDI devices
%description output-midi
Output plugin for Q Light Controller that enables DMX output support for any
MIDI-capable device.

%files output-midi
%defattr(-,root,root) 
/usr/lib/qlc/output/libmidiout.so
