/*
  Q Light Controller
  sceneeditor.h

  Copyright (C) 2000, 2001, 2002 Heikki Junnila
                            2002 Stefan Krumm

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

#include "uic_sceneeditor.h"
#include "scene.h"
#include "types.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout; 
class QButtonGroup;
class QGroupBox;
class QLabel;
class QPushButton;
class QRadioButton;
class QPopupMenu;

class Device;

class SceneEditor : public UI_SceneEditor
{
  Q_OBJECT

 public:
  SceneEditor(QWidget* parent, const char* name = NULL);
  ~SceneEditor();

  void setDevice(t_device_id);
  void initMenu();

  Scene* currentScene();

 signals:
  void sceneActivated(Scene*);

 public slots:
  void slotSceneListContextMenu(QListBoxItem*, const QPoint&);
  void slotChannelChanged(t_channel, t_value, Scene::ValueType);

  void slotActivate();
  void slotNew();
  void slotStore();
  void slotRemove();
  void slotRename();

 protected:
  void fillFunctions();
  void selectFunction(t_function_id fid);
  void setStatusText(QString text, QColor color);
  void setScene(Scene* scene);

 protected:
  t_device_id m_deviceID;
  QPopupMenu* m_menu;
  Scene* m_tempScene;
};

#endif
