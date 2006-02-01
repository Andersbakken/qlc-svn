lib_LTLIBRARIES = libsoundtolight.la

INCLUDES = @QT_CFLAGS@ \
	-DFIXTURES=\"$(datadir)/$(FIXTURES)\" \
	-DPIXMAPS=\"$(datadir)/$(PIXMAPS)\" \
	-DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\" \
	-DPLUGINS=\"$(PLUGINS)\"

noinst_HEADERS = \
	uic_configuresoundtolight.h \
	\
	configuresoundtolight.h \
	soundtolight.h

libsoundtolight_la_SOURCES = \
	uic_configuresoundtolight.cpp \
	\
	moc_uic_configuresoundtolight.cpp \
	\
	soundtolight.cpp \
	configuresoundtolight.cpp \
	\
	moc_soundtolight.cpp \
	moc_configuresoundtolight.cpp

libsoundtolight_la_LIBADD = @QT_LIBS@

libdir = $(PLUGINS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
