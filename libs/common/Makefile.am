lib_LTLIBRARIES = libqlccommon.la

libqlccommon_la_CXXFLAGS  = @QT_CFLAGS@ -I$(top_srcdir)/libs
libqlccommon_la_CXXFLAGS += -DFIXTURES=\"$(datadir)/$(FIXTURES)\"
libqlccommon_la_CXXFLAGS += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
libqlccommon_la_CXXFLAGS += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
libqlccommon_la_CXXFLAGS += -DPLUGINS=\"$(PLUGINS)\"
libqlccommon_la_CXXFLAGS += -DVERSION=\"$(VERSION)\"

libqlccommon_la_LIBS  = @QT_LIBS@

noinst_HEADERS = \
	capability.h \
	configitem.h \
	deviceclass.h \
	documentbrowser.h \
	filehandler.h \
	inputplugin.h \
	ledbar.h \
	logicalchannel.h \
	minmax.h \
	outputplugin.h \
	plugin.h \
	qlcfixturemode.h \
	qlcfixturedef.h \
	qlcchannel.h \
	qlccapability.h \
	qlcimagepreview.h \
	qlcphysical.h \
	qlcworkspace.h \
	settings.h

libqlccommon_la_SOURCES = \
	capability.cpp \
	deviceclass.cpp \
	documentbrowser.cpp \
	filehandler.cpp \
	inputplugin.cpp \
	ledbar.cpp \
	logicalchannel.cpp \
	outputplugin.cpp \
	plugin.cpp \
	qlcfixturemode.cpp \
	qlcfixturedef.cpp \
	qlcchannel.cpp \
	qlccapability.cpp \
	qlcimagepreview.cpp \
	qlcphysical.cpp \
	qlcworkspace.cpp \
	settings.cpp \
	\
	moc_documentbrowser.cpp \
	moc_filehandler.cpp \
	moc_inputplugin.cpp \
	moc_ledbar.cpp \
	moc_outputplugin.cpp \
	moc_plugin.cpp \
	moc_qlcimagepreview.cpp \
	moc_qlcworkspace.cpp \
	moc_settings.cpp

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
