/*
  Q Light Controller
  functionmanager.h

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

#ifndef FUNCTIONMANAGER_H
#define FUNCTIONMANAGER_H

#include <qwidget.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include "types.h"

class QPopupMenu;
class QDockArea;
class QToolBar;
class QToolButton;
class QListView;
class QListViewItem;
class QHBoxLayout;
class QSplitter;
class QPushButton;

class Function;
class Device;

typedef QValueList<t_function_id> FunctionIDList;

class FunctionManager : public QWidget
{
	Q_OBJECT

public:
	FunctionManager(QWidget* parent, WFlags flags = NormalMode);
	~FunctionManager();

	// Second stage initialization
	void init();

	// Selected functions from selection mode
	int selection(FunctionIDList& list);

	// Result from selection mode; unset in normal mode
	int result() { return m_result; }

	// Set a function to be inactive to prevent making direct loops
	void setInactiveID(t_function_id id) { m_inactiveID = id; }

public:
	static const WFlags NormalMode = 0;
	static const WFlags SelectionMode = WShowModal | WType_Dialog |
						WStyle_DialogBorder;

protected:
	void closeEvent(QCloseEvent* e);

protected:
	// Init menu
	void initMenu();
	// Init toolbar
	void initToolbar();
	// Init fixture tree view
	void initFixtureTree();
	// Init function tree view
	void initFunctionTree();

	// Get available fixtures
	void updateFixtureTree();

	// Update the item's contents from the given function
	void updateFunctionItem(QListViewItem* item, Function* function);

	// Delete all currently selected functions
	void deleteSelectedFunctions();

	// Update menu/tool item's enabled status
	void updateMenuItems();

	// Copy the given function to the given device
	Function* copyFunction(t_function_id, t_device_id);

protected slots:
	void slotFixtureHeaderClicked(int section);
	void slotFunctionHeaderClicked(int section);

	void slotFixtureTreeSelectionChanged(QListViewItem* item);
	void slotFunctionTreeSelectionChanged();

	void slotFixtureTreeContextMenuRequested(QListViewItem* item,
					         const QPoint& pos, int col);
	void slotFunctionTreeContextMenuRequested(QListViewItem* item,
					         const QPoint& pos, int col);

	void slotFunctionTreeDoubleClicked(QListViewItem* item,
					   const QPoint& pos, int col);

	void slotBusActivated(int busID);

	void slotAddScene();
	void slotAddChaser();
	void slotAddCollection();
	void slotAddSequence();
	void slotAddEFX();

	void slotCut();
	void slotCopy();
	void slotPaste();
	int slotEdit();
	void slotDelete();

	void slotAddMenuCallback(int);
	void slotUpdateBusMenu();

	void slotSelectAll();

	void slotOKClicked();
	void slotCancelClicked();

signals:
	void closed();

protected:
	typedef enum _ClipboardAction
	{
		ClipboardNone = 0,
		ClipboardCut,
		ClipboardCopy
	} ClipboardAction;

	QPopupMenu* m_addMenu;
	QPopupMenu* m_editMenu;
	QPopupMenu* m_busMenu;

	QDockArea* m_dockArea;
	QToolBar* m_toolbar;

	QToolButton* m_addSceneButton;
	QToolButton* m_addChaserButton;
	QToolButton* m_addCollectionButton;
	QToolButton* m_addSequenceButton;
	QToolButton* m_addEFXButton;

	QToolButton* m_cutButton;
	QToolButton* m_copyButton;
	QToolButton* m_pasteButton;
	QToolButton* m_editButton;
	QToolButton* m_deleteButton;

	QListView* m_fixtureTree;
	QListView* m_functionTree;

	QSplitter* m_treeViewSplitter;

	QPtrList<QListViewItem> m_selectedFunctions;
	FunctionIDList m_clipboard;
	ClipboardAction m_clipboardAction;

	// Selection mode stuff
	bool m_selectionMode;
	int m_result;
	t_function_id m_inactiveID;
	FunctionIDList m_selection;
	QHBoxLayout* m_buttonLayout;
	QPushButton* m_ok;
	QPushButton* m_cancel;
};

#endif

