/*
  Q Light Controller
  advancedsceneeditor.h

  Copyright (C) 2000, 2001, 2002 Heikki Junnila

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

#ifndef ADVANCEDSCENEEDITOR_H
#define ADVANCEDSCENEEDITOR_H

#include "uic_advancedsceneeditor.h"

class DMXDevice;
class Scene;
class LogicalChannel;

class AdvancedSceneEditor : public UI_AdvancedSceneEditor
{
  Q_OBJECT
    
 public:
  AdvancedSceneEditor(QWidget* parent, const char* name = NULL);
  virtual ~AdvancedSceneEditor();

  void init();

 public slots:
  void slotOutputDeviceActivated(const QString &text);

  void slotAddSceneClicked();
  void slotRemoveSceneClicked();
  void slotEditSceneNameClicked();
  void slotSceneSelected(QListViewItem* item);

  void slotEditValueClicked();
  void slotContentsClicked(QListViewItem* item);
  void slotContentsDoubleClicked(QListViewItem*);

  void slotApplyClicked();
  void slotOKClicked();
  void slotCancelClicked();

 private:
  void updateSceneList();
  void updateChannelList();
  void setDirty(bool dirty);
  bool dirtyCheck();

 private:
  bool m_dirty; // Indicates whether we need to save changes (dirty) or not
  bool m_candidate; // New scene added

  DMXDevice* m_device;

  QListViewItem* m_deviceRoot;
  QListViewItem* m_deviceClassRoot;
  QListViewItem* m_currentSceneItem;

  Scene* m_scene;
  LogicalChannel* m_currentChannel;
};

#endif
