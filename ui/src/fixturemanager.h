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

#include <QWidget>

#include "function.h"
#include "fixture.h"
#include "app.h"

class QTreeWidgetItem;
class FixtureConsole;
class QTextBrowser;
class QTreeWidget;
class QTabWidget;
class QSplitter;
class QAction;

#define KXMLQLCFixtureManager "FixtureManager"
#define KXMLQLCFixtureManagerSplitterSize "SplitterSize"

class FixtureManager : public QWidget
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /** Get the monitor singleton instance. Can be NULL. */
    static FixtureManager* instance() {
        return s_instance;
    }

    /** Create an instance  with parent. Fails if s_instance != NULL. */
    static void create(QWidget* parent);

    ~FixtureManager();

protected:
    /** Protected constructor to prevent multiple instances. */
    FixtureManager(QWidget* parent, Qt::WindowFlags f = 0);

private:
    Q_DISABLE_COPY(FixtureManager)

protected:
    /** The singleton FixtureManager instance */
    static FixtureManager* s_instance;

    /********************************************************************
     * Doc signal handlers
     ********************************************************************/
public slots:
    /** Listen to document changes (after App::newDocument())*/
    void slotDocumentChanged(Doc* doc);

    /** Callback for Doc::fixtureAdded() signals */
    void slotFixtureAdded(t_fixture_id id);

    /** Callback for Doc::fixtureRemoved() signals */
    void slotFixtureRemoved(t_fixture_id id);

    /** Callback that listens to mode change signals */
    void slotModeChanged(Doc::Mode mode);

    /********************************************************************
     * Data view
     ********************************************************************/
public:
    /** Update the list of fixtures */
    void updateView();

protected:
    /** Get a QTreeWidgetItem whose fixture ID is $id */
    QTreeWidgetItem* fixtureItem(t_fixture_id id) const;

    /** Construct the list view and data view */
    void initDataView();

    /** Update a single fixture's data into a QTreeWidgetItem */
    void updateItem(QTreeWidgetItem* item, Fixture* fxt);

    /** Copy the given function into the given fixture */
    void copyFunction(Function* function, Fixture* fxt);

    /** Display an error message if fixture add fails */
    void addFixtureErrorMessage();

protected slots:
    /** Callback for fixture list selection changes */
    void slotSelectionChanged();

    /** Callback for mouse double clicks */
    void slotDoubleClicked(QTreeWidgetItem* item);

protected:
    /** Get a CSS style sheet & HTML header for fixture info */
    QString fixtureInfoStyleSheetHeader();

protected:
    QSplitter* m_splitter;
    QTreeWidget* m_tree;

    QTabWidget* m_tab;
    QTextBrowser* m_info;
    FixtureConsole* m_console;

    /********************************************************************
     * Menu & Toolbar & Actions
     ********************************************************************/
protected:
    /** Construct actions for toolbar & context menu */
    void initActions();

    /** Construct the toolbar */
    void initToolBar();

protected slots:
    void slotAdd();
    void slotRemove();
    void slotProperties();

    /** Callback for right mouse button clicks over a fixture item */
    void slotContextMenuRequested(const QPoint& pos);

protected:
    QAction* m_addAction;
    QAction* m_removeAction;
    QAction* m_propertiesAction;
    QAction* m_consoleAction;
};

#endif
