/*
  Q Light Controller
  outputmanager.h

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

#ifndef OUTPUTMANAGER_H
#define OUTPUTMANAGER_H

#include <QWidget>

#include "app.h"

class QTreeWidgetItem;
class QTreeWidget;
class OutputPatch;
class OutputMap;
class QToolBar;

class OutputManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputManager)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** Get the OutputManager singleton instance. Can be NULL. */
    static OutputManager* instance() {
        return s_instance;
    }

    /** Create an instance with parent. Fails if s_instance is not NULL. */
    static void create(QWidget* parent);

    /** Normal public destructor */
    virtual ~OutputManager();

protected:
    /** Protected constructor to prevent multiple instances. */
    OutputManager(QWidget* parent, Qt::WindowFlags flags = 0);

protected slots:
    void slotModeChanged(Doc::Mode mode);
    void slotDocumentChanged(Doc* doc);

protected:
    static OutputManager* s_instance;

    /*********************************************************************
     * Tree widget
     *********************************************************************/
public:
    /** Update the output mapping tree */
    void updateTree();

protected:
    /** Update the contents of an OutputPatch to an item */
    void updateItem(QTreeWidgetItem* item, OutputPatch* op, int universe);

protected slots:
    /** Updates the mapping tree */
    void slotPluginConfigurationChanged();

protected:
    QTreeWidget* m_tree;

    /*********************************************************************
     * Toolbar
     *********************************************************************/
protected slots:
    void slotEditClicked();

protected:
    QToolBar* m_toolbar;

};

#endif
