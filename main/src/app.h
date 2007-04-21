/*
  Q Light Controller
  app.h

  Copyright (C) 2000, 2001, 2002 Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef APP_H
#define APP_H

#include <qmainwindow.h>
#include <qstring.h>
#include <qptrlist.h>
#include "common/inputplugin.h"
#include "common/outputplugin.h"

class QWorkspace;
class QApp;
class QMessageBox;
class QMenuBar;
class QToolBar;
class QStatusBar;
class QPopupMenu;
class QPixmap;
class QToolButton;
class QFileDialog;
class QWidgetList;
class QLabel;
class QTimer;

class DeviceManagerView;
class Settings;
class Doc;
class InputPlugin;
class OutputPlugin;
class FunctionConsumer;
class VirtualConsole;
class Plugin;
class DummyInPlugin;
class DummyOutPlugin;
class DeviceClass;
class FunctionManager;
class BusProperties;
class DocumentBrowser;
class PluginManager;
class Monitor;

const QString KApplicationNameLong  = "Q Light Controller";
const QString KApplicationNameShort = "QLC";
const QString KApplicationVersion   = QString("Version ") + QString(VERSION);

//////////////////////////////////////////////////////////////////
// Class definition

class App : public QMainWindow
{
	Q_OBJECT
public:
	App();
	~App();

public:
	void init();
	void newDocument();
	void saveSettings();

	DeviceManagerView* deviceManagerView() { return m_dmView; }
	VirtualConsole* virtualConsole() { return m_virtualConsole; }
	QWorkspace* workspace() { return m_workspace; }
	Settings* settings() { return m_settings; }
	FunctionConsumer* functionConsumer() { return m_functionConsumer; }
	Doc* doc() { return m_doc; }
	Monitor* monitor() { return m_monitor; }
  
	//
	// Plugin Stuff
	//
	InputPlugin* inputPlugin() { return m_inputPlugin; }
	OutputPlugin* outputPlugin() { return m_outputPlugin; }
	QPtrList <Plugin> *pluginList() { return &m_pluginList; }
	Plugin* searchPlugin(QString name);
	Plugin* searchPlugin(QString name, Plugin::PluginType type);
	Plugin* searchPlugin(const t_plugin_id id);
	void addPlugin(Plugin*);
	void removePlugin(Plugin*);
	void initPlugins();
	bool probePlugin(QString path);
	
	//
	// Device class stuff
	//
	void initDeviceClasses();
	void addDeviceClass(DeviceClass*);
	QPtrList <DeviceClass> *deviceClassList() { return &m_deviceClassList; }
	DeviceClass* searchDeviceClass(const QString &manufacturer,
					const QString &model);

	//
	// Mode: operate or design
	//
	enum Mode { Operate, Design };
	bool mode() { return m_mode; }

	//
	// Value read & write
	//
	void initValues();
	void setValue(t_channel, t_value);
	t_value value(t_channel);
	void valueRange(t_channel address, t_value* values, t_channel num);
	
	bool isBlackOut() { return m_blackOut; }

public:
	//
	// Submasters
	//
	void initSubmasters();
	bool assignSubmaster(t_channel);
	bool resignSubmaster(t_channel);
	int hasSubmaster(t_channel);
	void resetSubmasters();
	void setSubmasterValue(t_channel, int);
	float submasterValue(t_channel);
	
private slots:
	void slotChangeInputPlugin(const QString& name);
	void slotChangeOutputPlugin(const QString& name);
	void slotPluginActivated(Plugin* plugin);

	void slotFlashBlackOutIndicator();

public slots:
	bool slotFileNew();
	void slotFileOpen();
	void slotFileSave();
	void slotFileSaveAs();
	void slotFileSettings();
	void slotFilePlugins();
	void slotPluginManagerClosed();
	void slotFileQuit();
	
	void slotViewDeviceManager();
	void slotDeviceManagerViewClosed();
	
	void slotViewVirtualConsole();
	void slotVirtualConsoleClosed();
	
	void slotViewFunctionManager();
	void slotFunctionManagerClosed();
	
	void slotViewBusProperties();
	void slotBusPropertiesClosed();
	
	void slotViewMonitor();
	void slotMonitorClosed();

	void slotWindowCascade();
	void slotWindowTile();
	
	void slotHelpIndex();
	void slotDocumentBrowserClosed();
	
	void slotHelpAbout();
	void slotHelpAboutQt();
	void slotHelpTooltips();
	
	void slotPanic();
	void slotSetDesignMode();
	void slotSetOperateMode();
	void slotSetMode();
	
	void slotToggleBlackOut();
	
signals:
	void modeChanged();

private:
	void initSettings();
	void initDoc();
	void initWorkspace();
	
	void initVirtualConsole();
	void initFunctionConsumer();
	
	void initMenuBar();
	void initStatusBar();
	void initToolBar();
	
	void createJoystickContents(QPtrList <QString> &list);
	
private slots:
	void slotWindowMenuCallback(int item);
	void slotRefreshMenus();
	
private:
	QPopupMenu* m_fileMenu;
	QPopupMenu* m_toolsMenu;
	QPopupMenu* m_modeMenu;
	QPopupMenu* m_windowMenu;
	QPopupMenu* m_helpMenu;
	
	QToolBar* m_toolbar;
	QToolButton* m_newTB;
	QToolButton* m_openTB;
	QToolButton* m_saveTB;
	QToolButton* m_dmTB;
	QToolButton* m_vcTB;
	QToolButton* m_ftTB;
	QToolButton* m_panicTB;
	QToolButton* m_modeTB;
	QToolButton* m_blackOutTB;
	QToolButton* m_monitorTB;
	
	Doc* m_doc;
	Settings* m_settings;
	DeviceManagerView* m_dmView;
	VirtualConsole* m_virtualConsole;
	QWorkspace* m_workspace;
	FunctionManager* m_functionManager;
	BusProperties* m_busProperties;
	DocumentBrowser* m_documentBrowser;
	PluginManager* m_pluginManager;
	Monitor* m_monitor;

	FunctionConsumer* m_functionConsumer;

	QLabel* m_modeIndicator;
	QLabel* m_blackOutIndicator;
	QTimer* m_blackOutIndicatorTimer;
	
	InputPlugin* m_inputPlugin;
	OutputPlugin* m_outputPlugin;
	DummyInPlugin* m_dummyInPlugin;
	DummyOutPlugin* m_dummyOutPlugin;
	QPtrList <Plugin> m_pluginList;
	
	// Main operating mode
	Mode m_mode;
	
	// Device classes
	QPtrList <DeviceClass> m_deviceClassList;
	
	t_value m_values[KChannelMax];
	float m_submasterValues[KChannelMax];
	int m_submasters[KChannelMax];
	
	bool m_blackOut;
	
protected:
	void closeEvent(QCloseEvent*);
	
private:
	static t_plugin_id NextPluginID;
};

#endif

