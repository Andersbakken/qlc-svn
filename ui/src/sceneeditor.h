/*
  Q Light Controller
  sceneeditor.h

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#ifndef SCENEEDITOR_H
#define SCENEEDITOR_H

#include <QWidget>
#include <QList>
#include <QMap>

#include "qlctypes.h"

#include "ui_sceneeditor.h"
#include "fixture.h"
#include "scene.h"

class QAction;

class SceneEditor : public QDialog, public Ui_SceneEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(SceneEditor)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    SceneEditor(QWidget* parent, Scene* scene);
    ~SceneEditor();

protected:
    void init();
    void fillBusCombo();
    void setSceneValue(const SceneValue& scv);

protected:
    bool m_initializing;

    /*********************************************************************
     * Common
     *********************************************************************/
protected slots:
    void slotNameEdited(const QString& name);
    void slotBusComboActivated(int index);

    void accept();
    void slotTabChanged(int tab);

    void slotEnableAll();
    void slotDisableAll();

    void slotEnableCurrent();
    void slotDisableCurrent();

    void slotCopy();
    void slotPaste();
    void slotCopyToAll();
    void slotColorTool();

protected:
    bool isColorToolAvailable();

protected:
    QAction* m_enableCurrentAction;
    QAction* m_disableCurrentAction;

    QAction* m_copyAction;
    QAction* m_pasteAction;
    QAction* m_copyToAllAction;
    QAction* m_colorToolAction;

    /*********************************************************************
     * General tab
     *********************************************************************/
protected:
    QTreeWidgetItem* fixtureItem(t_fixture_id fxi_id);
    QList <Fixture*> selectedFixtures() const;

    void addFixtureItem(Fixture* fixture);
    void removeFixtureItem(Fixture* fixture);

protected slots:
    void slotAddFixtureClicked();
    void slotRemoveFixtureClicked();

    /*********************************************************************
     * Fixture tabs
     *********************************************************************/
protected:
    FixtureConsole* fixtureConsole(Fixture* fixture);

    void addFixtureTab(Fixture* fixture);
    void removeFixtureTab(Fixture* fixture);

protected:
    int m_currentTab;

    /*********************************************************************
     * Scene
     *********************************************************************/
protected:
    Scene* m_scene;
    Scene* m_original;

    /** Values from a copied console */
    QList <SceneValue> m_copy;
};

#endif
