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

#include "dmxdevice.h"
#include "scene.h"
#include "uic_sceneeditor.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout; 
class QButtonGroup;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QRadioButton;

class SceneEditor : public UI_SceneEditor
{
  Q_OBJECT

 public:
  SceneEditor(DMXDevice* device, QWidget* parent = 0, const char* name = 0);
  ~SceneEditor();

 public slots:
  void slotSceneChanged();
  void slotSceneActivated( int nr );
  void slotHideClicked();
  void slotNewClicked();
  void slotSaveClicked();
  void slotClassRadio_clicked();
  void slotDeviceRadio_clicked();

 protected:
  DMXDevice* m_device;
  Scene* m_currentScene;
  QString m_deviceSource;

 protected:
  void m_selectFunctions(QList <Function> fl);
  void m_setStatusText(QString text, QColor color);
  void setScene(Scene* scene);
};

#endif // SCENEEDITOR_H
