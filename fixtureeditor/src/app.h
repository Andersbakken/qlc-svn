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

class Settings;
class DocumentBrowser;
class QLCFixture;
class QLCFixtureEditor;
class QLCWorkspace;

class DeviceClass;

const QString KApplicationNameLong  = "QLC Device Class Editor";
const QString KApplicationNameShort = "DCE";
const QString KApplicationVersion   = "Version 2.3";

//////////////////////////////////////////////////////////////////
// Class definition

class App : public QMainWindow
{
	Q_OBJECT

 public:
	App(Settings* settings);
	~App();

 public:
	void initView();
	void initSettings();

	QLCWorkspace* workspace() { return m_workspace; }
	Settings* settings() { return m_settings; }

 private slots:
	void slotEmpty();

	void slotFileNew();
	void slotFileOpen();
	void slotFileSave();
	void slotFileSaveAs();
	void slotFileQuit();

	void slotWindowCascade();
	void slotWindowTile();

	void slotRefreshWindowMenu();

	void slotHelpIndex();
	void slotDocumentBrowserClosed();
	void slotHelpAbout();
	void slotHelpAboutQt();

	void slotEditorClosed(QLCFixtureEditor* editor);

	void slotBackgroundChanged(const QString& path);

 private:
	/** Open an old .deviceclass file */
	DeviceClass* openLegacyFile(QString path);

	void initWorkspace();

	void initMenuBar();
	void initStatusBar();
	void initToolBar();

 private slots:
	void slotWindowMenuCallback(int item);

 private:
	QLCWorkspace* m_workspace;
	Settings* m_settings;
	DocumentBrowser* m_documentBrowser;

	QPopupMenu* m_fileMenu;
	QPopupMenu* m_toolsMenu;
	QPopupMenu* m_windowMenu;
	QPopupMenu* m_helpMenu;

	QToolBar* m_toolbar;
	QString m_lastPath;

 protected:
	void closeEvent(QCloseEvent*);
	bool event(QEvent* e);
};

#endif