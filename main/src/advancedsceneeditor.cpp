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
#include "dmxdevice.h"
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


#include <math.h>

extern App* _app;

AdvancedSceneEditor::AdvancedSceneEditor(QWidget* parent, Scene* scene)
  : UI_AdvancedSceneEditor(parent, "AdvancedSceneEditor", false)
{
  ASSERT(scene != NULL);

  m_scene = new Scene(scene);
  m_currentChannel = NULL;

  m_dirty = false;
}

AdvancedSceneEditor::~AdvancedSceneEditor()
{
}

void AdvancedSceneEditor::init()
{
  double w = (double) m_sceneContents->width();

  m_sceneContents->setColumnWidth(0, (int) floor(w * 0.07));
  m_sceneContents->setColumnWidth(1, (int) floor(w * 0.38));
  m_sceneContents->setColumnWidth(2, (int) floor(w * 0.35));
  m_sceneContents->setColumnWidth(3, (int) floor(w * 0.1));
  m_sceneContents->setColumnWidth(4, (int) floor(w * 0.1));

  m_sceneNameEdit->setText(m_scene->name());
  m_sceneNameEdit->setSelection(0, m_scene->name().length());

  updateChannelList();

  setDirty(false);
}

void AdvancedSceneEditor::slotSnapshotClicked()
{
  if (m_scene == NULL)
    {
      return;
    }

  for (unsigned i = 0; i < m_scene->device()->deviceClass()->channels()->count(); i++)
    {
      DMXChannel* channel = m_scene->device()->dmxChannel(i);
      SceneValueType type = Fade;

      if (channel->status() == DMXChannel::On)
	{
	  type = Fade;
	}
      else
	{
	  type = NoSet;
	}

      m_scene->set(i, channel->value(), type);
    }

  updateChannelList();

  setDirty(true);
}

void AdvancedSceneEditor::slotChannelsContextMenuRequested(QListViewItem* item,
							   const QPoint &pos, int col)
{
  if (m_scene == NULL)
    {
      return;
    }

  switch (col)
    {
    case 0:
      break;
      
    case 1:
      break;

    case 2:
      invokePresetMenu(pos);
      break;

    case 3:
      invokeValueMenu(pos);
      break;

    case 4:
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
      connect(menu, SIGNAL(activated(int)), this, SLOT(slotPresetMenuActivated(int)));
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

  m_sceneContents->currentItem()->setText(2, c->name());

  QString s;
  s.setNum((int) floor((c->lo() + c->hi()) / 2));
  m_sceneContents->currentItem()->setText(3, s);

  setDirty(true);
}

void AdvancedSceneEditor::invokeValueMenu(const QPoint &pos)
{
  QPopupMenu* menu = new QPopupMenu;
  QList <QPopupMenu> deleteList;

  menu->insertItem("Value", INT_MAX);
  menu->insertSeparator();
  connect(menu, SIGNAL(activated(int)), this, SLOT(slotValueMenuActivated(int)));

  for (unsigned int i = 0; i < 256; i += 16)
    {
      QPopupMenu* sub = new QPopupMenu();
      deleteList.append(sub);

      QString top;
      top.sprintf("%d - %d", i, i + 15);

      for (unsigned int j = 0; j < 16; j++)
	{
	  QString num;
	  num.setNum(i + j);
	  sub->insertItem(num, i + j);
	}
      
      menu->insertItem(top, sub);
      connect(sub, SIGNAL(activated(int)), this, SLOT(slotValueMenuActivated(int)));
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

  if (value == INT_MAX)
    {
      slotEditValueClicked();
    }
  else
    {
      int channel = m_currentChannel->channel();
      
      m_scene->set(channel, value, m_scene->channelValue(channel).type);
      
      QString s;
      s.setNum(value);
      m_sceneContents->currentItem()->setText(3, s);
      
      Capability* c = m_currentChannel->searchCapability(value);
      if (c == NULL)
	{
	  s = QString("<Unknown>");
	}
      else
	{
	  s = c->name();
	}
      
      m_sceneContents->currentItem()->setText(2, s);
      setDirty(true);
    }
}

void AdvancedSceneEditor::invokeTypeMenu(const QPoint &pos)
{
  QPopupMenu* menu = new QPopupMenu;

  menu->insertItem("Type");
  menu->insertSeparator();
  menu->insertItem("Fade", (int) Fade);
  menu->insertItem("Set", (int) Set);
  menu->insertItem("NoSet", (int) NoSet);

  connect(menu, SIGNAL(activated(int)), this, SLOT(slotTypeMenuActivated(int)));
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

  int channel = m_currentChannel->channel();

  m_scene->set(channel, m_scene->channelValue(channel).value, (SceneValueType) type);

  switch (type)
    {
    case Fade:
      m_sceneContents->currentItem()->setText(4, "Fade");
      setDirty(true);
      break;

    case Set:
      m_sceneContents->currentItem()->setText(4, "Set");
      setDirty(true);
      break;

    case NoSet:
      m_sceneContents->currentItem()->setText(4, "NoSet");
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
      if (QMessageBox::information(this, "Advanced Scene Editor",
				   "Do you want to LOSE changes?",
				   QMessageBox::Yes, QMessageBox::Cancel)
	  == QMessageBox::Yes)
	{
	  if (m_scene != NULL)
	    {
	      delete m_scene;
	      m_scene = NULL;
	    }
	  
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
  int ch = item->text(0).toInt();

  if (m_scene->device() != NULL)
    {
      m_currentChannel = m_scene->device()->deviceClass()->channels()->at(ch);
    }
  else
    {
      ASSERT(false);
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

  Scene* s = (Scene*) _app->doc()->searchFunction(m_scene->id());
  ASSERT(s != NULL);

  _app->doc()->setModified(true);
  setDirty(false);

  s->copyFrom(m_scene);
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

  EditSceneValue* esv = new EditSceneValue((QWidget*) this, m_currentChannel, value);

  if (esv->exec() == QDialog::Accepted)
    {
      SceneValueType type;

      if (esv->type() == QString("Set"))
	{
	  type = Set;
	}
      else if (esv->type() == QString("Fade"))
	{
	  type = Fade;
	}
      else
	{
	  type = NoSet;
	}
      
      m_scene->set(ch, esv->value(), type);
      setDirty(true);
    }

  delete esv;

  updateChannelList();

  for (QListViewItem* item = m_sceneContents->firstChild(); item != NULL; item = item->nextSibling())
    {
      if (item->text(0).toInt() == ch)
	{
	  m_sceneContents->setSelected(item, true);
	  break;
	}
    }
}

void AdvancedSceneEditor::slotDeviceFunctionsListChanged(unsigned long fid)
{
  // If the currently edited scene was deleted, exit immediately
  if (m_scene && m_scene->id() == fid)
    {
      slotCancelClicked();
    }
}

void AdvancedSceneEditor::slotDeviceClassFunctionsListChanged(unsigned long fid)
{
  // If the currently edited scene was deleted, exit immediately
  if (m_scene && m_scene->id() == fid)
    {
      slotCancelClicked();
    }
}

void AdvancedSceneEditor::updateChannelList()
{
  QList <LogicalChannel> *cl = NULL;
  QString num;
  QString val;
  QString cap;

  m_sceneContents->clear();
  m_currentChannel = NULL;

  if (m_scene->device() != NULL)
    {
      cl = m_scene->device()->deviceClass()->channels();
    }
  else
    {
      ASSERT(false);
    }

  for (LogicalChannel* ch = cl->first(); ch != NULL; ch = cl->next())
    {
      unsigned char value = m_scene->channelValue(ch->channel()).value;
      
      Capability* c = ch->searchCapability(value);
      
      if (c == NULL)
	{
	  cap = QString("<Unknown>");
	}
      else
	{
	  cap = c->name();
	}
      
      num.sprintf("%03d", ch->channel());
      val.sprintf("%03d", m_scene->channelValue(ch->channel()).value);
      new QListViewItem(m_sceneContents, num, ch->name(), cap, val, m_scene->valueTypeString(ch->channel()));
    }
}
