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

#include <math.h>

extern App* _app;

AdvancedSceneEditor::AdvancedSceneEditor(QWidget* parent, const char* name)
  : UI_AdvancedSceneEditor(parent, name, false)
{
  m_device = NULL;
  m_scene = NULL;
  m_currentChannel = NULL;

  m_dirty = false;
}

AdvancedSceneEditor::~AdvancedSceneEditor()
{
}

void AdvancedSceneEditor::init()
{
  m_addSceneButton->setPixmap(_app->settings()->pixmapPath() + QString("add.xpm"));
  m_removeSceneButton->setPixmap(_app->settings()->pixmapPath() + QString("remove.xpm"));

  double w = (double) m_sceneContents->width();

  m_sceneContents->setColumnWidth(0, (int) floor(w * 0.07));
  m_sceneContents->setColumnWidth(1, (int) floor(w * 0.38));
  m_sceneContents->setColumnWidth(2, (int) floor(w * 0.35));
  m_sceneContents->setColumnWidth(3, (int) floor(w * 0.1));
  m_sceneContents->setColumnWidth(4, (int) floor(w * 0.1));

  QList <DMXDevice> *dl = _app->doc()->deviceList();

  for (DMXDevice* d = dl->first(); d != NULL; d = dl->next())
    {
      m_deviceCombo->insertItem(d->name());
    }

  if (m_deviceCombo->count() > 0)
    {
      m_deviceCombo->setCurrentItem(0);
      slotOutputDeviceActivated(m_deviceCombo->currentText());
    }

  setDirty(false);
}

void AdvancedSceneEditor::slotSnapshotClicked()
{
  if (m_scene == NULL)
    {
      return;
    }

  for (unsigned short i = 0; i < m_device->deviceClass()->channels()->count(); i++)
    {
      DMXChannel* channel = m_device->dmxChannel(i);
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

  QPoint p = m_sceneContents->mapFromGlobal(pos);
  int index = m_sceneContents->header()->sectionAt(p.x());

  switch (index)
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

  menu->insertItem("Value");
  menu->insertSeparator();

  for (unsigned int i = 0; i < 255; i++)
    {
      QString num;
      num.setNum(i);
      menu->insertItem(num, i);
    }

  connect(menu, SIGNAL(activated(int)), this, SLOT(slotValueMenuActivated(int)));
  menu->exec(pos);
  disconnect(menu);

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
      int result = QMessageBox::information(this, "Advanced Scene Editor",
					    "Do you want to LOSE changes?",
					    QMessageBox::Yes,QMessageBox::Cancel);
      if (result == QMessageBox::Yes)
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

void AdvancedSceneEditor::updateSceneList()
{
  m_sceneList->clear();
  m_sceneContents->clear();

  m_scene = NULL;
  m_currentChannel = NULL;
  m_currentSceneItem = NULL;

  if (m_device == NULL)
    {
      return;
    }

  m_deviceRoot = new QListViewItem(m_sceneList, "Device");
  m_deviceRoot->setOpen(true);

  QList <Function> *fl = m_device->functions();

  for (Function* f = fl->first(); f != NULL; f = fl->next())
    {
      if (f->type() == Function::Scene)
	{
	  new QListViewItem(m_deviceRoot, f->name());
	}
    }

  m_deviceClassRoot = new QListViewItem(m_sceneList, "Device Class");
  m_deviceClassRoot->setOpen(true);

  fl = m_device->deviceClass()->functions();

  for (Function* f = fl->first(); f != NULL; f = fl->next())
    {
      if (f->type() == Function::Scene)
	{
	  new QListViewItem(m_deviceClassRoot, f->name());
	}
    }

  setDirty(false);
}

void AdvancedSceneEditor::slotAddSceneClicked()
{
  bool ok = false;
  bool device = true;
  QString prompt, caption;

  if (m_sceneList->currentItem() == NULL)
    {
      return;
    }
  else if (m_sceneList->currentItem() == m_deviceRoot ||
	   m_sceneList->currentItem()->parent() == m_deviceRoot)
    {
      caption = QString("Adding new scene for DEVICE");
      prompt = QString("Enter new scene name for device ") + m_device->name();
      device = true;
    }
  else
    {
      caption = QString("Adding new scene for DEVICE CLASS");
      prompt = QString("Enter new scene name for device class ") +
	m_device->deviceClass()->manufacturer() + QString(" ") +
	m_device->deviceClass()->model();
      device = false;
    }

  QString text = QInputDialog::getText(caption, prompt, QLineEdit::Normal,
				       QString::null, &ok, this );
  if (ok && text.isEmpty() == false)
    {
      if (device == true)
	{
	  Scene* s = new Scene();
	  s->setName(text);
	  m_device->addFunction(s);
	}
      else
	{
	  Scene* s = new Scene();
	  s->setName(text);
	  m_device->deviceClass()->addFunction(s);
	}

      setDirty(false);
    }
  else
    {
    }
}

void AdvancedSceneEditor::slotRemoveSceneClicked()
{
  QListViewItem* item = m_sceneList->currentItem();

  if (item == NULL || m_scene == NULL)
    {
      return;
    }

  if (dirtyCheck() == false)
    {
      return;
    }

  if (item->parent() == m_deviceRoot)
    {
      Scene* s = (Scene*) m_device->searchFunctionById(m_scene->id());
      ASSERT(s != NULL);
      m_device->removeFunction((Function*) s);

      delete m_scene;
      m_scene = NULL;

      m_currentSceneItem = NULL;
    }
  else if (item->parent() == m_deviceClassRoot)
    {
      Scene* s = (Scene*) m_device->deviceClass()->searchFunctionById(m_scene->id());
      ASSERT(s != NULL);
      m_device->deviceClass()->removeFunction((Function*) s);

      delete m_scene;
      m_scene = NULL;

      m_currentSceneItem = NULL;
    }
}

void AdvancedSceneEditor::slotEditSceneNameClicked()
{
  if (m_scene == NULL)
    {
      return;
    }

  bool ok = false;

  QString text = QInputDialog::getText(QString("Edit Scene Name"), 
				       QString("Enter Scene Name"), 
				       QLineEdit::Normal,
				       m_scene->name(), &ok, this );
  if (ok && text.isEmpty() == false)
    {
      m_scene->setName(text);
      m_sceneList->currentItem()->setText(0, text);
      setDirty(true);
    }
}

void AdvancedSceneEditor::slotSceneDoubleClicked(QListViewItem* item)
{
  slotSceneSelected(item);
  slotEditSceneNameClicked();
}

void AdvancedSceneEditor::slotSceneSelected(QListViewItem* item)
{
  if (item->parent() == NULL)
    {
      m_scene = NULL;
      m_sceneContents->clear();
      return;
    }

  if (dirtyCheck() == false)
    {
      m_sceneList->setCurrentItem(m_currentSceneItem);
      return;
    }

  m_currentSceneItem = item;

  if (m_scene != NULL)
    {
      delete m_scene;
      m_scene = NULL;
    }

  ASSERT(m_device != NULL);

  if (item->parent() == m_deviceRoot)
    {
      m_scene = new Scene((Scene*) m_device->searchFunction(item->text(0)));
    }
  else if (item->parent() == m_deviceClassRoot)
    {
      m_scene = new Scene((Scene*) m_device->deviceClass()->searchFunction(item->text(0)));
    }
  else
    {
      ASSERT(false);
    }

  updateChannelList();
}

void AdvancedSceneEditor::slotContentsClicked(QListViewItem* item)
{
  ASSERT(m_device != NULL);

  int ch = item->text(0).toInt();

  m_currentChannel = m_device->deviceClass()->channels()->at(ch);
}

void AdvancedSceneEditor::slotContentsDoubleClicked(QListViewItem* item)
{
  slotContentsClicked(item);

  slotEditValueClicked();
}

void AdvancedSceneEditor::slotApplyClicked()
{
  ASSERT(m_device != NULL);

  if (m_scene == NULL)
    {
      return;
    }

  //
  // Save the name of the function and whether it is a device class or device function
  // so that we can select it after scene list update
  //
  QString sceneName = m_scene->name();
  bool dcFunction = (m_scene->deviceClass() != NULL) ? true : false;

  if (m_scene->deviceClass() != NULL)
    {
      Scene* s = (Scene*) m_device->deviceClass()->searchFunctionById(m_scene->id());
      ASSERT(s != NULL);
      
      m_device->deviceClass()->functions()->remove(s);
      delete s;
      
      m_device->deviceClass()->functions()->append(m_scene);

      m_device->deviceClass()->saveToFile();
      setDirty(false);
    }
  else if (m_scene->device() != NULL)
    {
      Scene* s = (Scene*) m_device->searchFunctionById(m_scene->id());
      ASSERT(s != NULL);
      
      m_device->functions()->remove(s);
      delete s;
      
      m_device->functions()->append(m_scene);

      _app->doc()->setModified(true);
      setDirty(false);
    }
  else
    {
      ASSERT(false);
    }

  //
  // Clear scene list and fetch new items
  //
  updateSceneList();

  //
  // Select the function that was selected before we pressed apply
  //
  if (dcFunction == true)
    {
      for (QListViewItem* item = m_deviceClassRoot->firstChild(); item != NULL; item = item->nextSibling())
	{
	  if (item->text(0) == sceneName)
	    {
	      m_sceneList->setSelected(item, true);
	      slotSceneSelected(item);
	      break;
	    }
	}
    }
  else
    {
      for (QListViewItem* item = m_deviceRoot->firstChild(); item != NULL; item = item->nextSibling())
	{
	  if (item->text(0) == sceneName)
	    {
	      m_sceneList->setSelected(item, true);
	      slotSceneSelected(item);
	      break;
	    }
	}
    }

}

void AdvancedSceneEditor::slotOKClicked()
{
  slotApplyClicked();
  close();
  emit closed();
}

void AdvancedSceneEditor::slotCancelClicked()
{
  if (dirtyCheck() == false)
    {
      return;
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

void AdvancedSceneEditor::slotOutputDeviceActivated(const QString &text)
{
  if (m_device != NULL)
    {
      QString device = m_device->name();
      
      if (dirtyCheck() == false)
	{
	  m_deviceCombo->setCurrentText(device);
	  m_sceneList->setCurrentItem(m_currentSceneItem);
	  return;
	}
    }

  m_device = _app->doc()->searchDevice(text);
  ASSERT(m_device != NULL);

  connect(m_device, SIGNAL(functionsListChanged(const QString&, bool)), this, SLOT(slotDeviceFunctionsListChanged(const QString&, bool)));
  connect(m_device->deviceClass(), SIGNAL(functionsListChanged(const QString&, bool)), this, SLOT(slotDeviceClassFunctionsListChanged(const QString&, bool)));

  m_deviceClassEdit->setText(m_device->deviceClass()->manufacturer() +
			     QString (" ") + m_device->deviceClass()->model());

  updateSceneList();
}

void AdvancedSceneEditor::slotDeviceFunctionsListChanged(const QString &name, bool add)
{
  if (add == true)
    {
      new QListViewItem(m_deviceRoot, QString(name));
    }
  else
    {
      // If the currently edited scene was deleted, update everything; it's easier
      if (m_scene && m_scene->name() == name)
	{
	  updateSceneList();
	}
      else
	{
	  // If some other function (not current scene) was deleted, remove only that one
	  for (QListViewItem* item = m_deviceRoot->firstChild(); item != NULL; item = item->nextSibling())
	    {
	      if (item->text(0) == name)
		{
		  delete item;
		  break;
		}
	    }
	}
    }
}

void AdvancedSceneEditor::slotDeviceClassFunctionsListChanged(const QString &name, bool add)
{
  if (add == true)
    {
      new QListViewItem(m_deviceClassRoot, QString(name));
    }
  else
    {
      // If the currently edited scene was deleted, update everything; it's easier
      if (m_scene && m_scene->name() == name)
	{
	  updateSceneList();
	}
      else
	{
	  // If some other function (not current scene) was deleted, remove only that one
	  for (QListViewItem* item = m_deviceClassRoot->firstChild(); item != NULL; item = item->nextSibling())
	    {
	      if (item->text(0) == name)
		{
		  delete item;
		  break;
		}
	    }
	}
    }
}

void AdvancedSceneEditor::updateChannelList()
{
  m_sceneContents->clear();
  m_currentChannel = NULL;

  if (m_scene == NULL)
    {
      return;
    }

  QString num;
  QString val;
  QString cap;

  QList <LogicalChannel> *cl = m_device->deviceClass()->channels();
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
      val.setNum(m_scene->channelValue(ch->channel()).value);
      new QListViewItem(m_sceneContents, num, ch->name(), cap, val, m_scene->valueTypeString(ch->channel()));
    }
}
