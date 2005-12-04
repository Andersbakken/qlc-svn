bin_PROGRAMS = dce

INCLUDES = @QT_CFLAGS@ \
	-DFIXTURES=\"$(datadir)/$(FIXTURES)\" \
	-DPIXMAPS=\"$(datadir)/$(PIXMAPS)\" \
	-DDOCUMENTS=\"$(prefix)/$(DOCUMENTS)/$(PACKAGE)\" \
	-DPLUGINS=\"$(PLUGINS)\"

dce_SOURCES = \
	src/uic_deviceclasseditor.cpp \
	src/uic_editpresetvalue.cpp \
	\
	src/moc_uic_deviceclasseditor.cpp \
	src/moc_uic_editpresetvalue.cpp \
	\
	src/aboutbox.cpp \
	src/app.cpp \
	src/deviceclasseditor.cpp \
	src/editpresetvalue.cpp \
	src/main.cpp \
	\
	src/moc_aboutbox.cpp \
	src/moc_app.cpp \
	src/moc_deviceclasseditor.cpp \
	src/moc_editpresetvalue.cpp \
	\
	../main/src/capability.cpp \
	../main/src/deviceclass.cpp \
	../main/src/logicalchannel.cpp \
	../main/src/settings.cpp \
	\
	../main/src/moc_capability.cpp \
	../main/src/moc_deviceclass.cpp \
	../main/src/moc_logicalchannel.cpp \
	../main/src/moc_settings.cpp

dce_LDADD = ../libs/common/libqlccommon.la @QT_LIBS@

# MOCS
moc_%.cpp: %.h
	moc $< -o $@

# UIC headers
uic_%.h: %.ui
	uic $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	uic -impl $< $*.ui -o $@
