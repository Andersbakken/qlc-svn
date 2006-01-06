lib_LTLIBRARIES = libqlccommon.la

INCLUDES = @QT_CFLAGS@ \
	-DFIXTURES=\"$(datadir)/$(FIXTURES)\" \
	-DPIXMAPS=\"$(datadir)/$(PIXMAPS)\" \
	-DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\" \
	-DPLUGINS=\"$(PLUGINS)\"

noinst_HEADERS = \
	configitem.h \
	documentbrowser.h \
	filehandler.h \
	ledbar.h \
	minmax.h \
	outputplugin.h \
	plugin.h \
	settings.h

libqlccommon_la_SOURCES = \
	documentbrowser.cpp \
	filehandler.cpp \
	ledbar.cpp \
	plugin.cpp \
	outputplugin.cpp \
	settings.cpp \
	\
	moc_filehandler.cpp \
	moc_ledbar.cpp \
	moc_plugin.cpp \
	moc_outputplugin.cpp \
	moc_documentbrowser.cpp \
	moc_settings.cpp

libqlccommon_la_LIBADD = @QT_LIBS@

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
