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

#include <qlistbox.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qbuttongroup.h>

extern App* _app;

AdvancedSceneEditor::AdvancedSceneEditor(QWidget* parent, const char* name)
  : UI_AdvancedSceneEditor(parent, name, true)
{
  m_device = NULL;
}

AdvancedSceneEditor::~AdvancedSceneEditor()
{
}

void AdvancedSceneEditor::init()
{
  m_addSceneButton->setPixmap(_app->settings()->pixmapPath() + QString("add.xpm"));
  m_removeSceneButton->setPixmap(_app->settings()->pixmapPath() + QString("remove.xpm"));

  double w = (double) m_sceneContents->width();

  m_sceneContents->setColumnWidth(0, (int) (w * 0.05));
  m_sceneContents->setColumnWidth(1, (int) (w * 0.5));
  m_sceneContents->setColumnWidth(2, (int) (w * 0.3));
  m_sceneContents->setColumnWidth(3, (int) (w * 0.145));

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
}

void AdvancedSceneEditor::slotUpdateSceneList()
{
  m_sceneList->clear();
  m_sceneContents->clear();

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
}

void AdvancedSceneEditor::slotAddSceneClicked()
{
}

void AdvancedSceneEditor::slotRemoveSceneClicked()
{
}

void AdvancedSceneEditor::slotClearValuesClicked()
{
}

void AdvancedSceneEditor::slotContentsClicked(QListViewItem* item)
{
}

void AdvancedSceneEditor::slotStoreSceneInGroupClicked(int item)
{
  switch (item)
    {
    case 0:
      break;

    case 1:
      break;

    default:
      break;
    }
}

void AdvancedSceneEditor::slotOutputDeviceActivated(const QString &text)
{
  m_device = _app->doc()->searchDevice(text);
  
  m_deviceClassEdit->setText(m_device->deviceClass()->manufacturer() +
			     QString (" ") + m_device->deviceClass()->model());

  slotUpdateSceneList();
}

void AdvancedSceneEditor::slotSceneNameTextChanged(const QString &text)
{
}

void AdvancedSceneEditor::slotSceneSelected(QListViewItem* item)
{
  ASSERT(m_device != NULL);
  if (item->parent() == NULL)
    {
      return;
    }

  m_sceneContents->clear();
  
  Scene* s = NULL;

  if (item->parent() == m_deviceRoot)
    {
      s = (Scene*) m_device->searchFunction(item->text(0));
    }
  else if (item->parent() == m_deviceClassRoot)
    {
      s = (Scene*) m_device->deviceClass()->searchFunction(item->text(0));
    }
  else
    {
      ASSERT(false);
    }

  QString num;
  QString cap;

  QList <LogicalChannel> *cl = m_device->deviceClass()->channels();
  for (LogicalChannel* c = cl->first(); c != NULL; c = cl->next())
    {
      cap = c->searchCapability(s->channelValue(c->channel()).value)->name();
      num.setNum(c->channel());
      new QListViewItem(m_sceneContents, num, c->name(), cap);
    }

}
