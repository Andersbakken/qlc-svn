bin_PROGRAMS = qlc

INCLUDES = @QT_CFLAGS@ \
	-DFIXTURES=\"$(datadir)/$(FIXTURES)\" \
	-DPIXMAPS=\"$(datadir)/$(PIXMAPS)\" \
	-DDOCUMENTS=\"$(prefix)/$(DOCUMENTS)/$(PACKAGE)\" \
	-DPLUGINS=\"$(PLUGINS)\"

qlc_SOURCES = \
	src/uic_advancedsceneeditor.cpp \
	src/uic_assignhotkey.cpp \
	src/uic_busproperties.cpp \
	src/uic_chasereditor.cpp \
	src/uic_consolechannel.cpp \
	src/uic_devicelist.cpp \
	src/uic_deviceproperties.cpp \
	src/uic_dmxaddresstool.cpp \
	src/uic_editscenevalue.cpp \
	src/uic_editsettingskey.cpp \
	src/uic_efxeditor.cpp \
	src/uic_functioncollectioneditor.cpp \
	src/uic_functiontree.cpp \
	src/uic_newdevice.cpp \
	src/uic_patterngenerator.cpp \
	src/uic_sceneeditor.cpp \
	src/uic_sequenceeditor.cpp \
	src/uic_settings.cpp \
	src/uic_vcbuttonproperties.cpp \
	src/uic_vcdocksliderproperties.cpp \
	src/uic_vcdockslider.cpp \
	src/uic_vcframeproperties.cpp \
	src/uic_vcxypadproperties.cpp \
	src/uic_virtualconsoleproperties.cpp \
	\
	src/moc_uic_advancedsceneeditor.cpp \
	src/moc_uic_assignhotkey.cpp \
	src/moc_uic_busproperties.cpp \
	src/moc_uic_chasereditor.cpp \
	src/moc_uic_consolechannel.cpp \
	src/moc_uic_devicelist.cpp \
	src/moc_uic_deviceproperties.cpp \
	src/moc_uic_dmxaddresstool.cpp \
	src/moc_uic_editscenevalue.cpp \
	src/moc_uic_editsettingskey.cpp \
	src/moc_uic_efxeditor.cpp \
	src/moc_uic_functioncollectioneditor.cpp \
	src/moc_uic_functiontree.cpp \
	src/moc_uic_newdevice.cpp \
	src/moc_uic_patterngenerator.cpp \
	src/moc_uic_sceneeditor.cpp \
	src/moc_uic_sequenceeditor.cpp \
	src/moc_uic_settings.cpp \
	src/moc_uic_vcbuttonproperties.cpp \
	src/moc_uic_vcdocksliderproperties.cpp \
	src/moc_uic_vcdockslider.cpp \
	src/moc_uic_vcframeproperties.cpp \
	src/moc_uic_vcxypadproperties.cpp \
	src/moc_uic_virtualconsoleproperties.cpp \
	\
	src/aboutbox.cpp \
	src/advancedsceneeditor.cpp \
	src/app.cpp \
	src/assignhotkey.cpp \
	src/bus.cpp \
	src/busproperties.cpp \
	src/capability.cpp \
	src/chaser.cpp \
	src/chasereditor.cpp \
	src/consolechannel.cpp \
	src/deviceclass.cpp \
	src/deviceconsole.cpp \
	src/device.cpp \
	src/devicelist.cpp \
	src/devicemanagerview.cpp \
	src/deviceproperties.cpp \
	src/dmxaddresstool.cpp \
	src/doc.cpp \
	src/documentbrowser.cpp \
	src/dummyoutplugin.cpp \
	src/editscenevalue.cpp \
	src/efx.cpp \
	src/efxeditor.cpp \
	src/eventbuffer.cpp \
	src/floatingedit.cpp \
	src/functioncollection.cpp \
	src/functioncollectioneditor.cpp \
	src/functionconsumer.cpp \
	src/function.cpp \
	src/functiontree.cpp \
	src/generatorarea.cpp \
	src/imagecontentspreview.cpp \
	src/keybind.cpp \
	src/listboxiditem.cpp \
	src/logicalchannel.cpp \
	src/main.cpp \
	src/monitor.cpp \
	src/newdevice.cpp \
	src/newinputdevice.cpp \
	src/patterngenerator.cpp \
	src/scene.cpp \
	src/sceneeditor.cpp \
	src/sequence.cpp \
	src/sequenceeditor.cpp \
	src/settings.cpp \
	src/settingsui.cpp \
	src/vcbutton.cpp \
	src/vcbuttonproperties.cpp \
	src/vcdockarea.cpp \
	src/vcdockslider.cpp \
	src/vcdocksliderproperties.cpp \
	src/vcframe.cpp \
	src/vcframeproperties.cpp \
	src/vclabel.cpp \
	src/vcxypad.cpp \
	src/vcxypadproperties.cpp \
	src/virtualconsole.cpp \
	src/virtualconsoleproperties.cpp \
	src/xychannelunit.cpp \
	\
	src/moc_aboutbox.cpp \
	src/moc_advancedsceneeditor.cpp \
	src/moc_app.cpp \
	src/moc_assignhotkey.cpp \
	src/moc_bus.cpp \
	src/moc_busproperties.cpp \
	src/moc_capability.cpp \
	src/moc_chaser.cpp \
	src/moc_chasereditor.cpp \
	src/moc_consolechannel.cpp \
	src/moc_deviceclass.cpp \
	src/moc_deviceconsole.cpp \
	src/moc_device.cpp \
	src/moc_devicelist.cpp \
	src/moc_devicemanagerview.cpp \
	src/moc_deviceproperties.cpp \
	src/moc_dmxaddresstool.cpp \
	src/moc_doc.cpp \
	src/moc_documentbrowser.cpp \
	src/moc_dummyoutplugin.cpp \
	src/moc_editscenevalue.cpp \
	src/moc_efx.cpp \
	src/moc_efxeditor.cpp \
	src/moc_eventbuffer.cpp \
	src/moc_floatingedit.cpp \
	src/moc_functioncollection.cpp \
	src/moc_functioncollectioneditor.cpp \
	src/moc_functionconsumer.cpp \
	src/moc_function.cpp \
	src/moc_functiontree.cpp \
	src/moc_generatorarea.cpp \
	src/moc_imagecontentspreview.cpp \
	src/moc_keybind.cpp \
	src/moc_listboxiditem.cpp \
	src/moc_logicalchannel.cpp \
	src/moc_monitor.cpp \
	src/moc_newdevice.cpp \
	src/moc_newinputdevice.cpp \
	src/moc_patterngenerator.cpp \
	src/moc_scene.cpp \
	src/moc_sceneeditor.cpp \
	src/moc_sequence.cpp \
	src/moc_sequenceeditor.cpp \
	src/moc_settings.cpp \
	src/moc_settingsui.cpp \
	src/moc_vcbutton.cpp \
	src/moc_vcbuttonproperties.cpp \
	src/moc_vcdockarea.cpp \
	src/moc_vcdockslider.cpp \
	src/moc_vcdocksliderproperties.cpp \
	src/moc_vcframe.cpp \
	src/moc_vcframeproperties.cpp \
	src/moc_vclabel.cpp \
	src/moc_vcxypad.cpp \
	src/moc_vcxypadproperties.cpp \
	src/moc_virtualconsole.cpp \
	src/moc_virtualconsoleproperties.cpp \
	src/moc_xychannelunit.cpp

qlc_LDADD = ../libs/common/libqlccommon.la @QT_LIBS@

# MOCS
moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@

install-data-local:
	install -o root -g root -m 644 $(top_srcdir)/xpm/Q.xpm $(datadir)/pixmaps/qlc.xpm
	desktop-file-install qlc.desktop --vendor=qlc
