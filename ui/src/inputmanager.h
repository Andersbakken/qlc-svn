/*
  Q Light Controller
  inputmanager.h

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

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <QWidget>

#include "qlctypes.h"
#include "app.h"

class QTreeWidgetItem;
class QTreeWidget;
class QToolBar;
class QTimer;

class InputPatch;
class InputMap;

class InputManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(InputManager)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    /** Get the InputManager singleton instance. Can be NULL. */
    static InputManager* instance() { return s_instance; }

    /** Create an instance with parent. Fails if s_instance is not NULL. */
    static void create(QWidget* parent);

    /** Normal public destructor */
    virtual ~InputManager();

protected:
    /** Protected constructor to prevent multiple instances. */
    InputManager(QWidget* parent, Qt::WindowFlags flags = 0);

protected slots:
    void slotModeChanged(Doc::Mode mode);
    void slotDocumentChanged(Doc* doc);

protected:
    static InputManager* s_instance;

    /*************************************************************************
     * Tree widget
     *************************************************************************/
public:
    /** Update the input mapping tree */
    void updateTree();

protected:
    /** Update the contents of an input patch to an item */
    void updateItem(QTreeWidgetItem* item, InputPatch* patch, quint32 universe);

protected slots:
    /** Updates the tree whwn plugin configuration changes */
    void slotPluginConfigurationChanged();

    /** Listens to input data and displays a small icon to indicate a
        working connection between a plugin and an input device. */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

    /** Hides the small icon after a while ^^ */
    void slotTimerTimeout();

protected:
    QTreeWidget* m_tree;
    QTimer* m_timer;

    /*************************************************************************
     * Toolbar
     *************************************************************************/
protected slots:
    void slotEditClicked();

protected:
    QToolBar* m_toolbar;
};

#endif
