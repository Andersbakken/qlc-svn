include(../variables.pri)

TEMPLATE = subdirs
TARGET = profiles

profiles.files += Behringer-BCF2000.qxi
profiles.files += Enttec-PlaybackWing.qxi
profiles.files += Enttec-ShortcutWing.qxi
profiles.files += Korg-nanoKONTROL.qxi
profiles.files += Logitech-WingManAttack2.qxi
profiles.files += Novation-KS5.qxi
profiles.files += ShowTec-ShowMaster24.qxi

profiles.path = $$INSTALLROOT/$$INPUTPROFILEDIR
INSTALLS += profiles
