/*
  Q Light Controller
  advancedsceneeditor.cpp

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

#include "advancedsceneeditor.h"
#include "app.h"
#include "doc.h"
#include "function.h"
#include "device.h"
#include "settings.h"
#include "logicalchannel.h"
#include "capability.h"
#include "scene.h"
#include "editscenevalue.h"

#include <qlistbox.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qbuttongroup.h>
#include <qtooltip.h>
#include <qinputdialog.h>
#include <qapplication.h>
#include <qfont.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <qmessagebox.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

extern App* _app;

const int KColumnNumber  ( 0 );
const int KColumnChannel ( 1 );
const int KColumnPreset  ( 2 );
const int KColumnValue   ( 3 );
const int KColumnType    ( 4 );

AdvancedSceneEditor::AdvancedSceneEditor(QWidget* parent, Scene* scene)
  : UI_AdvancedSceneEditor(parent, "AdvancedSceneEditor", false)
{
  ASSERT(scene);
  m_original = scene;

  m_scene = new Scene();
  m_scene->copyFrom(scene, scene->device());
  m_currentChannel = NULL;

  m_dirty = false;
}

AdvancedSceneEditor::~AdvancedSceneEditor()
{
}

void AdvancedSceneEditor::init()
{
  m_sceneNameEdit->setText(m_scene->name());
  m_sceneNameEdit->setSelection(0, m_scene->name().length());

  updateChannelList();

  setDirty(false);
}

void AdvancedSceneEditor::slotChannelsContextMenuRequested(QListViewItem* item,
							   const QPoint &pos,
							   int col)
{
  if (m_scene == NULL || m_sceneContents->isRenaming())
    {
      return;
    }

  switch (col)
    {
    case KColumnNumber:
      break;
      
    case KColumnChannel:
      break;

    case KColumnPreset:
      invokePresetMenu(pos);
      break;

    case KColumnValue:
      invokeValueMenu(pos);
      break;

    case KColumnType:
      invokeTypeMenu(pos);
      break;

    default:
      break;
    }
}

void AdvancedSceneEditor::invokePresetMenu(const QPoint &pos)
{
  QPopupMenu* menu = new QPopupMenu;

  menu->insertItem("Preset");
  menu->insertSeparator();

  int i = 0;
  for (Capability* c = m_currentChannel->capabilities()->first(); c != NULL; 
       c = m_currentChannel->capabilities()->next())
    {
      menu->insertItem(c->name(), i++);
    }

  if (i > 0)
    {
      connect(menu, SIGNAL(activated(int)), 
	      this, SLOT(slotPresetMenuActivated(int)));
    }

  menu->exec(pos);
  disconnect(menu);

  delete menu;
}

void AdvancedSceneEditor::slotPresetMenuActivated(int preset)
{
  if (preset < 0)
    {
      return;
    }

  Capability* c = m_currentChannel->capabilities()->at(preset);
  ASSERT(c);

  int channel = m_currentChannel->channel();
  int value = (int) floor((c->lo() + c->hi()) / 2);

  m_scene->set(channel, value, m_scene->channelValue(channel).type);

  m_sceneContents->currentItem()->setText(KColumnPreset, c->name());

  QString s;
  s.setNum(value);
  m_sceneContents->currentItem()->setText(KColumnValue, s);

  setDirty(true);
}

void AdvancedSceneEditor::invokeValueMenu(const QPoint &pos)
{
  QPopupMenu* menu = new QPopupMenu;
  QPtrList <QPopupMenu> deleteList;

  menu->insertItem("Value", KNoID);
  menu->insertSeparator();
  connect(menu, SIGNAL(activated(int)), 
	  this, SLOT(slotValueMenuActivated(int)));

  menu->insertItem("0", 0);
  menu->insertItem("255", 255);
  for (t_value i = 0; i != 255; i += 15)
    {
      QPopupMenu* sub = new QPopupMenu();
      deleteList.append(sub);

      QString top;
      top.sprintf("%d - %d", i+1, i + 15);

      for (t_value j = 1; j < 16; j++)
	{
	  QString num;
	  num.setNum(i + j);
	  sub->insertItem(num, i + j);
	}
      
      menu->insertItem(top, sub);
      connect(sub, SIGNAL(activated(int)), 
	      this, SLOT(slotValueMenuActivated(int)));
    }

  menu->exec(pos);

  while (deleteList.isEmpty() == false)
    {
      delete deleteList.take(0);
    }

  delete menu;
}

void AdvancedSceneEditor::slotValueMenuActivated(int value)
{
  if (m_scene == NULL)
    {
      return;
    }

  if (value < 0)
    {
      return;
    }

  if (value == KNoID)
    {
      slotEditValueClicked();
    }
  else
    {
      int channel = m_currentChannel->channel();
      
      m_scene->set(channel, value, m_scene->channelValue(channel).type);
      
      QString s;
      s.sprintf("%.3d", value);
      m_sceneContents->currentItem()->setText(KColumnValue, s);
      
      Capability* c = m_currentChannel->searchCapability(value);
      if (c == NULL)
	{
	  s = QString("<Unknown>");
	}
      else
	{
	  s = c->name();
	}
      
      m_sceneContents->currentItem()->setText(KColumnPreset, s);
      setDirty(true);
    }
}

void AdvancedSceneEditor::invokeTypeMenu(const QPoint &pos)
{
  QPopupMenu* menu = new QPopupMenu;

  menu->insertItem("Type");
  menu->insertSeparator();
  menu->insertItem("Fade", Scene::Fade);
  menu->insertItem("Set", Scene::Set);
  menu->insertItem("NoSet", Scene::NoSet);

  t_channel channel = m_currentChannel->channel();
  SceneValue v = m_scene->channelValue(channel);

  if (v.type == Scene::Fade)
    {
      menu->setItemChecked(Scene::Fade, true);
    }
  else if (v.type == Scene::Set)
    {
      menu->setItemChecked(Scene::Set, true);
    }
  else
    {
      menu->setItemChecked(Scene::NoSet, true);
    }

  connect(menu, SIGNAL(activated(int)), 
	  this, SLOT(slotTypeMenuActivated(int)));

  menu->exec(pos);
  disconnect(menu);

  delete menu;
}

void AdvancedSceneEditor::slotTypeMenuActivated(int type)
{
  if (m_scene == NULL)
    {
      return;
    }

  t_channel channel = m_currentChannel->channel();

  m_scene->set(channel, m_scene->channelValue(channel).value,
	       static_cast<Scene::ValueType> (type));

  switch (type)
    {
    case Scene::Fade:
      m_sceneContents->currentItem()->setText(KColumnType, "Fade");
      setDirty(true);
      break;

    case Scene::Set:
      m_sceneContents->currentItem()->setText(KColumnType, "Set");
      setDirty(true);
      break;

    case Scene::NoSet:
      m_sceneContents->currentItem()->setText(KColumnType, "NoSet");
      setDirty(true);
      break;

    default:
      break;
    }
}

void AdvancedSceneEditor::setDirty(bool dirty)
{
  m_applyButton->setEnabled(dirty);
  m_dirty = dirty;
}

bool AdvancedSceneEditor::dirtyCheck()
{
  if (m_dirty == true)
    {
      int result = QMessageBox::information(this, "Advanced Scene Editor",
					    "Do you want to save changes?",
					    QMessageBox::Yes,
					    QMessageBox::No,
					    QMessageBox::Cancel);
      if (result == QMessageBox::Yes)
	{
	  m_original->copyFrom(m_scene, m_original->device());
	  setDirty(false);
	  return true;
	}
      else if (result == QMessageBox::No)
	{
	  setDirty(false);
	  return true;
	}
      else
	{
	  return false;
	}
    }
  else
    {
      return true;
    }
}

void AdvancedSceneEditor::slotSceneNameTextChanged(const QString& text)
{
  if (m_scene->name() != text)
    {
      m_scene->setName(text);

      setDirty(true);
    }
}

void AdvancedSceneEditor::slotContentsClicked(QListViewItem* item)
{
  if (item)
    {
      int ch = item->text(KColumnNumber).toInt() - 1;
      t_device_id did = m_scene->device();
      
      if (did != KNoID)
	{
	  m_currentChannel = 
	    _app->doc()->device(did)->deviceClass()->channels()->at(ch);
	}
      else
	{
	  assert(false);
	}
    }
}

void AdvancedSceneEditor::slotContentsDoubleClicked(QListViewItem* item)
{
  slotContentsClicked(item);

  slotEditValueClicked();
}

void AdvancedSceneEditor::slotApplyClicked()
{
  if (m_scene == NULL)
    {
      return;
    }

  _app->doc()->setModified(true);
  setDirty(false);

  m_original->copyFrom(m_scene, m_original->device());
}

void AdvancedSceneEditor::slotOKClicked()
{
  slotApplyClicked();

  if (m_scene != NULL)
    {
      delete m_scene;
      m_scene = NULL;
    }

  close();
  emit closed();
}

void AdvancedSceneEditor::slotCancelClicked()
{
  if (dirtyCheck() == false)
    {
      return;
    }

  if (m_scene != NULL)
    {
      delete m_scene;
      m_scene = NULL;
    }

  close();
  emit closed();
}

void AdvancedSceneEditor::slotEditValueClicked()
{
  if (m_currentChannel == NULL)
    {
      return;
    }

  int ch = m_currentChannel->channel();

  SceneValue value = m_scene->channelValue(ch);

  EditSceneValue* esv = new EditSceneValue((QWidget*) this, 
					   m_currentChannel, value);

  if (esv->exec() == QDialog::Accepted)
    {
      Scene::ValueType type;

      if (esv->type() == QString("Set"))
	{
	  type = Scene::Set;
	}
      else if (esv->type() == QString("Fade"))
	{
	  type = Scene::Fade;
	}
      else
	{
	  type = Scene::NoSet;
	}
      
      m_scene->set(ch, esv->value(), type);
      setDirty(true);
    }

  delete esv;

  updateChannelList();

  for (QListViewItem* item = m_sceneContents->firstChild(); 
       item != NULL; item = item->nextSibling())
    {
      if (item->text(KColumnNumber).toInt() - 1 == ch)
	{
	  m_sceneContents->setSelected(item, true);
	  break;
	}
    }
}

void AdvancedSceneEditor::slotDeviceFunctionsListChanged(t_function_id fid)
{
  // If the currently edited scene was deleted, exit immediately
  if (m_scene && m_scene->id() == fid)
    {
      slotCancelClicked();
    }
}

void AdvancedSceneEditor::updateChannelList()
{
  QPtrList <LogicalChannel> *cl = NULL;
  QString num;
  QString val;
  QString cap;
  QListViewItem* item = NULL;

  m_sceneContents->clear();
  m_currentChannel = NULL;

  t_device_id did = m_scene->device();
  if (did != KNoID)
    {
      cl = _app->doc()->device(did)->deviceClass()->channels();
    }
  else
    {
      ASSERT(false);
    }

  for (LogicalChannel* ch = cl->first(); ch != NULL; ch = cl->next())
    {
      t_value value = m_scene->channelValue(ch->channel()).value;
      
      Capability* c = ch->searchCapability(value);
      
      if (c == NULL)
	{
	  cap = QString("<Unknown>");
	}
      else
	{
	  cap = c->name();
	}
      
      num.sprintf("%03d", ch->channel() + 1);
      val.sprintf("%03d", m_scene->channelValue(ch->channel()).value);
      item = new QListViewItem(m_sceneContents, num, ch->name(), cap, val, 
			       m_scene->valueTypeString(ch->channel()));
    }
}
