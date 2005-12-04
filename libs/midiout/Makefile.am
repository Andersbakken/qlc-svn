lib_LTLIBRARIES = libmidiout.la

INCLUDES = @QT_CFLAGS@ \
	-DFIXTURES=\"$(datadir)/$(FIXTURES)\" \
	-DPIXMAPS=\"$(datadir)/$(PIXMAPS)\" \
	-DDOCUMENTS=\"$(prefix)/$(DOCUMENTS)/$(PACKAGE)\" \
	-DPLUGINS=\"$(PLUGINS)\"

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
	moc $< -o $@

# UIC headers
uic_%.h: %.ui
	uic $< -o $@

# UIC sources 
uic_%.cpp: uic_%.h
	uic -impl $< $*.ui -o $@
