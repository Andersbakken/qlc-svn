/*
  Q Light Controller
  sequenceeditor.h
  
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

#ifndef SEQUENCEEDITOR_H
#define SEQUENCEEDITOR_H

#include <qptrlist.h>
#include <qlayout.h>
#include <qevent.h>

#include "uic_sequenceeditor.h"
#include "scene.h"
#include "types.h"
#include "consolechannel.h"

class Scene;

class SequenceEditor : public UI_SequenceEditor
{
  Q_OBJECT

 public:
  SequenceEditor(QWidget* parent, const char* name = NULL);
  ~SequenceEditor();

  void init();

  void setDevice(t_device_id);

 public slots:
  void slotInsert();
  void slotRemove();
  void slotRaise();
  void slotLower();
  void slotOKClicked();
  void slotCancelClicked();

  void slotChannelChanged(t_channel, t_value, Scene::ValueType);

 protected:
  void resizeEvent(QResizeEvent* e);

 protected:
  Scene* m_tempScene;
  t_channel m_channels;

  QPtrList <ConsoleChannel> m_unitList;
  QPtrList <QSpacerItem> m_spacerList;
};

#endif
