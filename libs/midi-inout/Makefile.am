lib_LTLIBRARIES = libmidiinout.la

INCLUDES = @QT_CFLAGS@ \
	-DFIXTURES=\"$(datadir)/$(FIXTURES)\" \
	-DPIXMAPS=\"$(datadir)/$(PIXMAPS)\" \
	-DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\" \
	-DPLUGINS=\"$(PLUGINS)\"

noinst_HEADERS = \
	uic_configuremidi-inout.h \
	\
	configuremidi-inout.h \
	midi-inout.h

libmidiinout_la_SOURCES = \
	uic_configuremidi-inout.cpp \
	\
	moc_uic_configuremidi-inout.cpp \
	\
	midi-inout.cpp \
	configuremidi-inout.cpp \
	\
	moc_midi-inout.cpp \
	moc_configuremidi-inout.cpp

libmidiinout_la_LIBADD = @QT_LIBS@

libdir = $(PLUGINS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
