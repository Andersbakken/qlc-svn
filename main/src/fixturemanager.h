/*
  Q Light Controller
  fixturemanager.h
  
  Copyright (c) Heikki Junnila

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

#ifndef FIXTUREMANAGER_H
#define FIXTUREMANAGER_H

#include "fixture.h"
#include "function.h"

#include <qwidget.h>

class QToolBar;
class QToolButton;
class QVBoxLayout;
class QCloseEvent;
class QDockArea;
class QSplitter;
class QListView;
class QListViewItem;
class QTextView;

class Fixture;

#define KEY_FIXTURE_MANAGER_OPEN "FixtureManagerOpen"
#define KEY_FIXTURE_MANAGER_X "FixtureManagerRectX"
#define KEY_FIXTURE_MANAGER_Y "FixtureManagerRectY"
#define KEY_FIXTURE_MANAGER_W "FixtureManagerRectW"
#define KEY_FIXTURE_MANAGER_H "FixtureManagerRectH"
#define KEY_FIXTURE_MANAGER_SPLITTER "FixtureManagerSplitter"

class FixtureManager : public QWidget
{
	Q_OBJECT

 public:
	/** Constructor */
	FixtureManager(QWidget* parent);

	/** Destructor */
	~FixtureManager();

	/** Second-stage initialization */
	void initView();

 protected:
	/** Set the window title and set an icon */
	void initTitle();

	/** Construct the toolbar */
	void initToolBar();

	/** Construct the list view and data view */
	void initDataView();

	/** Update the list of fixtures */
	void updateView();
  
	/** Update a single fixture's data into a QListViewItem */
	void updateItem(QListViewItem* item, Fixture* fxt);

	void copyFunction(Function* function, Fixture* fxt);

 public slots:
	 /** Callback for Doc::fixtureAdded() signals */
	void slotFixtureAdded(t_fixture_id id);

	 /** Callback for Doc::fixtureRemoved() signals */
	void slotFixtureRemoved(t_fixture_id id);

	/** Callback that listens to App mode change signals */
	void slotModeChanged();

 protected slots:
	 /** Callback for menu/tool item to add a fixture */
	void slotAdd();

	 /** Callback for menu/tool item to remove a fixture */
	void slotRemove();

	 /** Callback for menu/tool item to clone a fixture */
	void slotClone();

	void slotProperties();
	void slotConsole();
	void slotAutoFunction();

	/** Callback for fixture list selection changes */
	void slotSelectionChanged(QListViewItem*);

	 /** Callback for mouse double clicks */
	void slotDoubleClicked(QListViewItem*);

	/** Callback for right mouse button clicks over a fixture item */
	void slotRightButtonClicked(QListViewItem*, const QPoint&, int);

	/** Callback for fixture list context menu activations */
	void slotMenuCallBack(int);

 protected:
	QVBoxLayout* m_layout;
	QToolBar* m_toolbar;
	QDockArea* m_dockArea;
	QSplitter* m_splitter;
	QListView* m_listView;
	QTextView* m_textView;

	QToolButton* m_addButton;
	QToolButton* m_cloneButton;
	QToolButton* m_removeButton;
	QToolButton* m_propertiesButton;
	QToolButton* m_consoleButton;

 signals:
	void closed();

 protected:
	void closeEvent(QCloseEvent*);
};

#endif
