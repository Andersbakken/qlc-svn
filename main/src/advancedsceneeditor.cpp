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

#include <math.h>

extern App* _app;

AdvancedSceneEditor::AdvancedSceneEditor(QWidget* parent, const char* name)
  : UI_AdvancedSceneEditor(parent, name, true)
{
  m_device = NULL;
  m_scene = NULL;
  m_currentChannel = NULL;

  m_dirty = false;
  m_candidate = false;
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
					    QMessageBox::Yes, QMessageBox::Cancel);
      if (result == QMessageBox::Yes)
	{
	  if (m_scene != NULL)
	    {
	      delete m_scene;
	      m_scene = NULL;
	    }
	  
	  m_candidate = false;
	  
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
	  m_scene = new Scene();
	  m_scene->setName(text);
	  m_scene->setDevice(m_device);

	  QListViewItem* item = new QListViewItem(m_deviceRoot, text);
	  m_sceneList->setCurrentItem(item);
	  m_currentSceneItem = item;
	}
      else
	{
	  m_scene = new Scene();
	  m_scene->setName(text);
	  m_scene->setDeviceClass(m_device->deviceClass());

	  QListViewItem* item = new QListViewItem(m_deviceClassRoot, text);
	  m_sceneList->setCurrentItem(item);
	  m_currentSceneItem = item;
	}

      m_candidate = true;
      setDirty(true);
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

  if (m_candidate == true)
    {
      delete m_scene;
      m_scene = NULL;

      delete m_sceneList->currentItem();
      m_currentSceneItem = NULL;

      m_candidate = false;

      setDirty(false);
    }
  else if (item->parent() == m_deviceRoot)
    {
      Scene* s = (Scene*) m_device->searchFunctionById(m_scene->id());
      ASSERT(s != NULL);
      m_device->functions()->remove(s);
      delete s;

      delete m_scene;
      m_scene = NULL;

      m_currentSceneItem = NULL;
    }
  else if (item->parent() == m_deviceClassRoot)
    {
      Scene* s = (Scene*) m_device->deviceClass()->searchFunctionById(m_scene->id());
      ASSERT(s != NULL);
      m_device->deviceClass()->functions()->remove(s);
      delete s;

      delete m_scene;
      m_scene = NULL;

      m_currentSceneItem = NULL;
    }
  
  updateSceneList();
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

  QString sceneName = m_scene->name();

  if (m_scene->deviceClass() != NULL)
    {
      if (m_candidate == true)
	{
	  m_device->deviceClass()->functions()->append(m_scene);
	  m_candidate = false;
	}
      else
	{
	  Scene* s = (Scene*) m_device->deviceClass()->searchFunctionById(m_scene->id());
	  ASSERT(s != NULL);

	  m_device->deviceClass()->functions()->remove(s);
	  delete s;

	  m_device->deviceClass()->functions()->append(m_scene);
	}

      m_device->deviceClass()->saveToFile();
      setDirty(false);
    }
  else if (m_scene->device() != NULL)
    {
      if (m_candidate == true)
	{
	  m_device->functions()->append(m_scene);
	  m_candidate = false;
	}
      else
	{
	  Scene* s = (Scene*) m_device->searchFunctionById(m_scene->id());
	  ASSERT(s != NULL);

	  m_device->functions()->remove(s);
	  delete s;

	  m_device->functions()->append(m_scene);
	}

      _app->doc()->setModified(true);
      setDirty(false);
    }
  else
    {
      ASSERT(false);
    }

  updateSceneList();

  for (QListViewItem* item = m_sceneList->firstChild(); item != NULL; item = item->nextSibling())
    {
      if (item->text(0) == sceneName)
	{
	  m_sceneList->setSelected(item, true);
	  slotSceneSelected(item);
	  break;
	}
    }
}

void AdvancedSceneEditor::slotOKClicked()
{
  slotApplyClicked();
  accept();
}

void AdvancedSceneEditor::slotCancelClicked()
{
  if (m_candidate == true)
    {
      delete m_scene;
      m_scene = NULL;
    }

  reject();
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

void AdvancedSceneEditor::slotOutputDeviceActivated(const QString &text)
{
  m_device = _app->doc()->searchDevice(text);

  m_deviceClassEdit->setText(m_device->deviceClass()->manufacturer() +
			     QString (" ") + m_device->deviceClass()->model());

  updateSceneList();
}

void AdvancedSceneEditor::slotSceneSelected(QListViewItem* item)
{
  if (item->parent() == NULL)
    {
      m_scene = NULL;
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
