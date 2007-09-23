bin_PROGRAMS = qlc

qlc_CXXFLAGS  = @QT_CFLAGS@ -I$(top_srcdir)/libs
qlc_CXXFLAGS +=	-DFIXTURES=\"$(datadir)/$(FIXTURES)\"
qlc_CXXFLAGS += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
qlc_CXXFLAGS += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
qlc_CXXFLAGS += -DPLUGINS=\"$(PLUGINS)\"
qlc_CXXFLAGS += -DVERSION=\"$(VERSION)\"

qlc_LDADD  = @QT_LIBS@
qlc_LDADD += $(top_srcdir)/libs/common/libqlccommon.la

noinst_HEADERS = \
	src/uic_addfixture.h \
	src/uic_advancedsceneeditor.h \
	src/uic_assignhotkey.h \
	src/uic_assignsliderhotkey.h \
	src/uic_busproperties.h \
	src/uic_chasereditor.h \
	src/uic_consolechannel.h \
	src/uic_dmxaddresstool.h \
	src/uic_editscenevalue.h \
	src/uic_editsettingskey.h \
	src/uic_efxeditor.h \
	src/uic_fixturelist.h \
	src/uic_fixtureproperties.h \
	src/uic_functioncollectioneditor.h \
	src/uic_sceneeditor.h \
	src/uic_settings.h \
	src/uic_vcbuttonproperties.h \
	src/uic_vcdocksliderproperties.h \
	src/uic_vcdockslider.h \
	src/uic_vcframeproperties.h \
	src/uic_vcsliderproperties.h \
	src/uic_vcxypadproperties.h \
	src/uic_virtualconsoleproperties.h \
	\
	src/aboutbox.h \
	src/addfixture.h \
	src/advancedsceneeditor.h \
	src/app.h \
	src/assignhotkey.h \
	src/assignsliderhotkey.h \
	src/bus.h \
	src/busproperties.h \
	src/chasereditor.h \
	src/chaser.h \
	src/configkeys.h \
	src/consolechannel.h \
	src/dmxaddresstool.h \
	src/doc.h \
	src/dummyinplugin.h \
	src/dummyoutplugin.h \
	src/editscenevalue.h \
	src/efxeditor.h \
	src/efx.h \
	src/eventbuffer.h \
	src/fixture.h \
	src/fixtureconsole.h \
	src/fixturelist.h \
	src/fixturemanager.h \
	src/fixtureproperties.h \
	src/floatingedit.h \
	src/functioncollectioneditor.h \
	src/functioncollection.h \
	src/functionconsumer.h \
	src/function.h \
	src/functionmanager.h \
	src/generatorarea.h \
	src/keybind.h \
	src/listboxiditem.h \
	src/monitor.h \
	src/pluginmanager.h \
	src/sceneeditor.h \
	src/scene.h \
	src/settingsui.h \
	src/sliderkeybind.h \
	src/vcbutton.h \
	src/vcbuttonproperties.h \
	src/vcdockarea.h \
	src/vcdockslider.h \
	src/vcdocksliderproperties.h \
	src/vcframe.h \
	src/vcframeproperties.h \
	src/vclabel.h \
	src/vcslider.h \
	src/vcsliderproperties.h \
	src/vcwidget.h \
	src/vcxypad.h \
	src/vcxypadproperties.h \
	src/virtualconsole.h \
	src/virtualconsoleproperties.h \
	src/xychannelunit.h

qlc_SOURCES = \
	src/uic_addfixture.cpp \
	src/uic_advancedsceneeditor.cpp \
	src/uic_assignhotkey.cpp \
	src/uic_assignsliderhotkey.cpp \
	src/uic_busproperties.cpp \
	src/uic_chasereditor.cpp \
	src/uic_consolechannel.cpp \
	src/uic_dmxaddresstool.cpp \
	src/uic_editscenevalue.cpp \
	src/uic_editsettingskey.cpp \
	src/uic_efxeditor.cpp \
	src/uic_fixturelist.cpp \
	src/uic_fixtureproperties.cpp \
	src/uic_functioncollectioneditor.cpp \
	src/uic_sceneeditor.cpp \
	src/uic_settings.cpp \
	src/uic_vcbuttonproperties.cpp \
	src/uic_vcdocksliderproperties.cpp \
	src/uic_vcdockslider.cpp \
	src/uic_vcframeproperties.cpp \
	src/uic_vcsliderproperties.cpp \
	src/uic_vcxypadproperties.cpp \
	src/uic_virtualconsoleproperties.cpp \
	\
	src/moc_uic_addfixture.cpp \
	src/moc_uic_advancedsceneeditor.cpp \
	src/moc_uic_assignhotkey.cpp \
	src/moc_uic_assignsliderhotkey.cpp \
	src/moc_uic_busproperties.cpp \
	src/moc_uic_chasereditor.cpp \
	src/moc_uic_consolechannel.cpp \
	src/moc_uic_dmxaddresstool.cpp \
	src/moc_uic_editscenevalue.cpp \
	src/moc_uic_editsettingskey.cpp \
	src/moc_uic_efxeditor.cpp \
	src/moc_uic_fixturelist.cpp \
	src/moc_uic_fixtureproperties.cpp \
	src/moc_uic_functioncollectioneditor.cpp \
	src/moc_uic_sceneeditor.cpp \
	src/moc_uic_settings.cpp \
	src/moc_uic_vcbuttonproperties.cpp \
	src/moc_uic_vcdocksliderproperties.cpp \
	src/moc_uic_vcdockslider.cpp \
	src/moc_uic_vcframeproperties.cpp \
	src/moc_uic_vcsliderproperties.cpp \
	src/moc_uic_vcxypadproperties.cpp \
	src/moc_uic_virtualconsoleproperties.cpp \
	\
	src/aboutbox.cpp \
	src/addfixture.cpp \
	src/advancedsceneeditor.cpp \
	src/app.cpp \
	src/assignhotkey.cpp \
	src/assignsliderhotkey.cpp \
	src/bus.cpp \
	src/busproperties.cpp \
	src/chaser.cpp \
	src/chasereditor.cpp \
	src/consolechannel.cpp \
	src/dmxaddresstool.cpp \
	src/doc.cpp \
	src/dummyinplugin.cpp \
	src/dummyoutplugin.cpp \
	src/editscenevalue.cpp \
	src/efx.cpp \
	src/efxeditor.cpp \
	src/eventbuffer.cpp \
	src/fixture.cpp \
	src/fixtureconsole.cpp \
	src/fixturelist.cpp \
	src/fixturemanager.cpp \
	src/fixtureproperties.cpp \
	src/floatingedit.cpp \
	src/functioncollection.cpp \
	src/functioncollectioneditor.cpp \
	src/functionconsumer.cpp \
	src/function.cpp \
	src/functionmanager.cpp \
	src/generatorarea.cpp \
	src/keybind.cpp \
	src/listboxiditem.cpp \
	src/main.cpp \
	src/monitor.cpp \
	src/pluginmanager.cpp \
	src/scene.cpp \
	src/sceneeditor.cpp \
	src/settingsui.cpp \
	src/sliderkeybind.cpp \
	src/vcbutton.cpp \
	src/vcbuttonproperties.cpp \
	src/vcdockarea.cpp \
	src/vcdockslider.cpp \
	src/vcdocksliderproperties.cpp \
	src/vcframe.cpp \
	src/vcframeproperties.cpp \
	src/vclabel.cpp \
	src/vcslider.cpp \
	src/vcsliderproperties.cpp \
	src/vcwidget.cpp \
	src/vcxypad.cpp \
	src/vcxypadproperties.cpp \
	src/virtualconsole.cpp \
	src/virtualconsoleproperties.cpp \
	src/xychannelunit.cpp \
	\
	src/moc_aboutbox.cpp \
	src/moc_addfixture.cpp \
	src/moc_advancedsceneeditor.cpp \
	src/moc_app.cpp \
	src/moc_assignhotkey.cpp \
	src/moc_assignsliderhotkey.cpp \
	src/moc_bus.cpp \
	src/moc_busproperties.cpp \
	src/moc_chaser.cpp \
	src/moc_chasereditor.cpp \
	src/moc_consolechannel.cpp \
	src/moc_dmxaddresstool.cpp \
	src/moc_doc.cpp \
	src/moc_dummyinplugin.cpp \
	src/moc_dummyoutplugin.cpp \
	src/moc_editscenevalue.cpp \
	src/moc_efx.cpp \
	src/moc_efxeditor.cpp \
	src/moc_eventbuffer.cpp \
	src/moc_fixture.cpp \
	src/moc_fixtureconsole.cpp \
	src/moc_fixturelist.cpp \
	src/moc_fixturemanager.cpp \
	src/moc_fixtureproperties.cpp \
	src/moc_floatingedit.cpp \
	src/moc_functioncollection.cpp \
	src/moc_functioncollectioneditor.cpp \
	src/moc_functionconsumer.cpp \
	src/moc_function.cpp \
	src/moc_functionmanager.cpp \
	src/moc_generatorarea.cpp \
	src/moc_keybind.cpp \
	src/moc_listboxiditem.cpp \
	src/moc_monitor.cpp \
	src/moc_pluginmanager.cpp \
	src/moc_scene.cpp \
	src/moc_sceneeditor.cpp \
	src/moc_settingsui.cpp \
	src/moc_sliderkeybind.cpp \
	src/moc_vcbutton.cpp \
	src/moc_vcbuttonproperties.cpp \
	src/moc_vcdockarea.cpp \
	src/moc_vcdockslider.cpp \
	src/moc_vcdocksliderproperties.cpp \
	src/moc_vcframe.cpp \
	src/moc_vcframeproperties.cpp \
	src/moc_vclabel.cpp \
	src/moc_vcslider.cpp \
	src/moc_vcsliderproperties.cpp \
	src/moc_vcwidget.cpp \
	src/moc_vcxypad.cpp \
	src/moc_vcxypadproperties.cpp \
	src/moc_virtualconsole.cpp \
	src/moc_virtualconsoleproperties.cpp \
	src/moc_xychannelunit.cpp

# MOCS
moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
