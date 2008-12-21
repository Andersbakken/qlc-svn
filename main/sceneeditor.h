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
#include "ui_sceneeditor.h"

#include "common/qlctypes.h"
#include "fixture.h"
#include "scene.h"

class SceneEditor : public QDialog, public Ui_SceneEditor
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	SceneEditor(QWidget* parent, Scene* scene);
	~SceneEditor();

private:
	Q_DISABLE_COPY(SceneEditor)

protected:
	void init();
	void setSceneValue(const SceneValue& scv);

protected:
	bool m_initializing;

	/*********************************************************************
	 * Common
	 *********************************************************************/
protected slots:
	void accept();

	/*********************************************************************
	 * General tab
	 *********************************************************************/
protected:
	QTreeWidgetItem* fixtureItem(t_fixture_id fxi_id);
	QList <Fixture*> selectedFixtures();

	void addFixtureItem(Fixture* fixture);
	void removeFixtureItem(Fixture* fixture);

protected slots:
	void slotAddFixtureClicked();
	void slotRemoveFixtureClicked();

	void slotEnableChannelsClicked();
	void slotDisableChannelsClicked();

	/*********************************************************************
	 * Fixture tabs
	 *********************************************************************/
protected:
	FixtureConsole* fixtureConsole(Fixture* fixture);

	void addFixtureTab(Fixture* fixture);
	void removeFixtureTab(Fixture* fixture);

protected slots:
	void slotValueChanged(t_fixture_id fxi_id, t_channel channel,
			      t_value value, bool enabled);

	/*********************************************************************
	 * Scene
	 *********************************************************************/
protected:
	Scene* m_scene;
	Scene* m_original;
};

#endif
