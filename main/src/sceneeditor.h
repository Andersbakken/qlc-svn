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

#include <qvariant.h>
#include <qwidget.h>

#include "common/types.h"
#include "uic_sceneeditor.h"
#include "scene.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout; 
class QButtonGroup;
class QGroupBox;
class QLabel;
class QPushButton;
class QRadioButton;
class QPopupMenu;
class ListBoxIDItem;

class Fixture;

class SceneEditor : public UI_SceneEditor
{
	Q_OBJECT

 public:
	SceneEditor(QWidget* parent);
	~SceneEditor();

	void setFixture(t_fixture_id id);
	void initMenu();

	Scene* currentScene();

 signals:
	void sceneActivated(SceneValue* values, t_channel channels);

 public slots:
	void slotSceneListContextMenu(QListBoxItem*, const QPoint&);
	void slotChannelChanged(t_channel, t_value, Scene::ValueType);

	void slotActivate();
	void slotNew();
	void slotStore();
	void slotRemove();
	void slotRename();

	void slotFunctionAdded(t_function_id);
	void slotFunctionRemoved(t_function_id);
	void slotFunctionChanged(t_function_id);

 protected:
	ListBoxIDItem* getItem(t_function_id id);
	void fillFunctions();
	void selectFunction(t_function_id fid);
	void setStatusText(QString text, QColor color);
	void setScene(Scene* scene);

 protected:
	t_fixture_id m_fixture;
	QPopupMenu* m_menu;
	Scene* m_tempScene;
};

#endif
