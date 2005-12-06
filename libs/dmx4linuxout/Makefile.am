lib_LTLIBRARIES = libdmx4linuxout.la

INCLUDES = @QT_CFLAGS@ \
	-DFIXTURES=\"$(datadir)/$(FIXTURES)\" \
	-DPIXMAPS=\"$(datadir)/$(PIXMAPS)\" \
	-DDOCUMENTS=\"$(prefix)/$(DOCUMENTS)/$(PACKAGE)\" \
	-DPLUGINS=\"$(PLUGINS)\"

libdmx4linuxout_la_SOURCES = \
	uic_configuredmx4linuxout.cpp \
	\
	moc_uic_configuredmx4linuxout.cpp \
	\
	dmx4linuxout.cpp \
	configuredmx4linuxout.cpp \
	\
	moc_dmx4linuxout.cpp \
	moc_configuredmx4linuxout.cpp

libdmx4linuxout_la_LIBADD = @QT_LIBS@

libdir = $(PLUGINS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources 
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
