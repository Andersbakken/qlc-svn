lib_LTLIBRARIES = libqlccommon.la

INCLUDES = @QT_CFLAGS@ \
	-DFIXTURES=\"$(datadir)/$(FIXTURES)\" \
	-DPIXMAPS=\"$(datadir)/$(PIXMAPS)\" \
	-DDOCUMENTS=\"$(prefix)/$(DOCUMENTS)/$(PACKAGE)\" \
	-DPLUGINS=\"$(PLUGINS)\"

libqlccommon_la_SOURCES = \
	filehandler.cpp \
	ledbar.cpp \
	plugin.cpp \
	outputplugin.cpp \
	\
	moc_filehandler.cpp \
	moc_ledbar.cpp \
	moc_plugin.cpp \
	moc_outputplugin.cpp	

libqlccommon_la_LIBADD = @QT_LIBS@

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
