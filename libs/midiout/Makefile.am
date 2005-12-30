lib_LTLIBRARIES = libmidiout.la

INCLUDES = @QT_CFLAGS@ \
	-DFIXTURES=\"$(datadir)/$(FIXTURES)\" \
	-DPIXMAPS=\"$(datadir)/$(PIXMAPS)\" \
	-DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\" \
	-DPLUGINS=\"$(PLUGINS)\"

noinst_HEADERS = \
	uic_configuremidiout.h \
	\
	configuremidiout.h \
	midiout.h

libmidiout_la_SOURCES = \
	uic_configuremidiout.cpp \
	\
	moc_uic_configuremidiout.cpp \
	\
	midiout.cpp \
	configuremidiout.cpp \
	\
	moc_midiout.cpp \
	moc_configuremidiout.cpp

libmidiout_la_LIBADD = @QT_LIBS@

libdir = $(PLUGINS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources 
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
