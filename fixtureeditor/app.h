/*
  Q Light Controller - Fixture Definition Editor
  app.h

  Copyright (C) Heikki Junnila

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

#include <QMainWindow>

class QToolBar;
class QAction;
class QMenu;

class QLCChannel;

const QString KApplicationName = "QLC Fixture Definition Editor";
const QString KApplicationVersion = "Version 3.0.0";

//////////////////////////////////////////////////////////////////
// Class definition

class App : public QMainWindow
{
	Q_OBJECT

public:
	App(QWidget* parent);
	~App();

protected:
	void loadDefaults();
	void saveDefaults();

	void closeEvent(QCloseEvent*);

protected:
	QString m_lastPath;

	/*********************************************************************
	 * Copy channel
	 *********************************************************************/
public:
	void setCopyChannel(QLCChannel* ch);
	QLCChannel* copyChannel() const;

protected:
	QLCChannel* m_copyChannel;

	/*********************************************************************
	 * Actions, toolbar & menubar
	 *********************************************************************/
protected:
	void initActions();
	void initMenuBar();
	void initToolBar();

protected:
	QAction* m_fileNewAction;
	QAction* m_fileOpenAction;
	QAction* m_fileSaveAction;
	QAction* m_fileSaveAsAction;
	QAction* m_fileQuitAction;

	QAction* m_helpIndexAction;
	QAction* m_helpAboutAction;
	QAction* m_helpAboutQtAction;

protected:
	QMenu* m_fileMenu;
	QMenu* m_helpMenu;
	QToolBar* m_toolBar;

protected slots:
	void slotFileNew();
	void slotFileOpen();
	void slotFileSave();
	void slotFileSaveAs();
	void slotFileQuit();

	void slotHelpIndex();
	void slotHelpAbout();
	void slotHelpAboutQt();
};

#endif
