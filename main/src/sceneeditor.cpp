/*
  Q Light Controller
  sceneeditor.cpp

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


#include "app.h"
#include "doc.h"
#include "settings.h"
#include "sceneeditor.h"
#include "function.h"
#include "deviceclass.h"
#include "dmxdevice.h"
#include "scene.h"
#include "dmxchannel.h"
#include "logicalchannel.h"
#include "channelui.h"
#include "listboxiditem.h"

#include <qlistbox.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <stdlib.h>
#include <unistd.h>
#include <qpopupmenu.h>
#include <qtoolbutton.h>

#include <iostream>
using namespace std;

extern App* _app;

#define MENU_ACTIVATE 1000
#define MENU_NEW      1001
#define MENU_STORE    1002
#define MENU_REMOVE   1003
#define MENU_RENAME   1004
#define MENU_HIDE     1005

static const QString KStatusStored = QString("stored");
static const QString KStatusUnchanged = QString("unchanged");
static const QString KStatusModified = QString("modified");

static const QColor KStatusColorStored = QColor(100, 255, 100);
static const QColor KStatusColorUnchanged = QColor(255, 255, 255);
static const QColor KStatusColorModified = QColor(255, 100, 100);

SceneEditor::SceneEditor(DMXDevice* device, QWidget* parent)
  : UI_SceneEditor(parent)
{
  m_device = device;
  m_menu = NULL;
}

SceneEditor::~SceneEditor()
{

}

void SceneEditor::init()
{
  m_menu = new QPopupMenu();
  connect(m_menu, SIGNAL(activated(int)), this, SLOT(slotMenuCallback(int)));

  m_menu->insertItem(QPixmap(_app->settings()->pixmapPath() + QString("key.xpm")),
		   "Activate", MENU_ACTIVATE);
  m_menu->insertSeparator();
  m_menu->insertItem(QPixmap(_app->settings()->pixmapPath() + QString("filenew.xpm")),
		   "New...", MENU_NEW);
  m_menu->insertItem(QPixmap(_app->settings()->pixmapPath() + QString("filesave.xpm")),
		   "Store", MENU_STORE);
  m_menu->insertItem(QPixmap(_app->settings()->pixmapPath() + QString("remove.xpm")),
		   "Remove", MENU_REMOVE);
  m_menu->insertItem(QPixmap(_app->settings()->pixmapPath() + QString("rename.xpm")),
		   "Rename...", MENU_RENAME);
  m_menu->insertSeparator();
  m_menu->insertItem(QPixmap(_app->settings()->pixmapPath() + QString("fileclose.xpm")),
		   "Hide Editor", MENU_HIDE);

  m_tools->setPopup(m_menu);

  fillFunctions();
}

void SceneEditor::slotSceneChanged()
{
  setStatusText(KStatusModified, KStatusColorModified);
}

void SceneEditor::slotSceneActivated(int nr)
{
  Scene* s = currentScene();

  if (s != NULL)
    {
      setScene(s);
    }

  setStatusText(KStatusUnchanged, QColor(255, 255, 255));
}


void SceneEditor::setScene(Scene* scene)
{
   ChannelUI* unit;
   QList <ChannelUI> ul = ((DMXDevice*) m_device)->getChannelUnitList();
   int n = 0;

   ASSERT(scene != NULL);

   for (unit = ul.first(); unit != NULL; unit = ul.next(), n++)
     {
       SceneValue value = scene->channelValue(n);
       if (value.type == Set || value.type == Fade)
	 {
	   unit->slotAnimateValueChange(value.value);
	   unit->setStatusButton(DMXChannel::On);
	 }
       else
	 {
	   unit->setStatusButton(DMXChannel::Off);
	 }
     }
}

void SceneEditor::slotSceneListContextMenu(QListBoxItem* item, const QPoint &point)
{
  m_menu->exec(point);
}

void SceneEditor::slotMenuCallback(int item)
{
  switch(item)
    {
    case MENU_ACTIVATE:
      slotSceneActivated(m_sceneList->currentItem());
      break;

    case MENU_NEW:
      newScene();
      break;

    case MENU_STORE:
      store();
      break;

    case MENU_REMOVE:
      remove();
      break;

    case MENU_RENAME:
      rename();
      break;

    case MENU_HIDE:
      hide();
      break;

    default:
      break;
    }
}

void SceneEditor::remove()
{
  Scene* s = currentScene();

  if (s == NULL)
    {
      return;
    }

  if (QMessageBox::warning(this, "Scene Editor", "Remove selected scene?",
			   QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
      m_device->removeFunction(s->id());
      fillFunctions();
    }
}

void SceneEditor::rename()
{
  bool ok = false;
  Scene* s = currentScene();

  if (s == NULL)
    {
      return;
    }

  QString text = QInputDialog::getText("Scene editor - Rename Scene",
				       "Enter scene name",
				       QLineEdit::Normal,
				       s->name(), &ok, this);
  if (ok && !text.isEmpty())
    {
      s->setName(text);
      fillFunctions();

      selectFunction(s->id());
    }
}

void SceneEditor::hide()
{
  for (unsigned i = parentWidget()->width(); i > ChannelUI::width() * m_device->deviceClass()->channels()->count(); i--)
    {
      parentWidget()->resize(i, height());
    }
}

void SceneEditor::newScene()
{
  bool ok = FALSE;
  QString text = QInputDialog::getText(tr("Scene editor - New Scene"),
				       tr("Enter scene name"),
				       QLineEdit::Normal,
				       QString::null, &ok, this);
  
  if (ok && !text.isEmpty())
    {
      Scene* sc = new Scene();
      sc->setName(text);
      sc->setDevice(m_device);
      for (unsigned int n = 0; n < m_device->deviceClass()->channels()->count(); n++)
	{
	  // Get values from device / HJu
	  if (m_device->dmxChannel(n)->status() == DMXChannel::On)
	    {
	      sc->set(n, m_device->dmxChannel(n)->value(), Set);
	    }
	  else
	    {
	      sc->clear(n);
	    }
	}
      
      // Save to device / HJu
      m_device->addFunction(sc);

      fillFunctions();
      selectFunction(sc->id());
    }
  
  setStatusText(KStatusStored, KStatusColorStored);
}

void SceneEditor::store()
{
  Scene* s = currentScene();
  if (s == NULL)
    {
      return;
    }

  QList <ChannelUI> ul = ((DMXDevice*) m_device)->getChannelUnitList();

  // Take values from device because it returns real values for
  // sure and they are unsigned chars and it is much simpler this way / HJu
  for (unsigned short i = 0; i < m_device->deviceClass()->channels()->count(); i++)
    {
      if (m_device->dmxChannel(i)->status() == DMXChannel::On)
	{
	  s->set(i, m_device->dmxChannel(i)->value(), Set);
	}
      else
	{
	  s->clear(i);
	}
    }

  setStatusText(KStatusStored, KStatusColorStored);
}

Scene* SceneEditor::currentScene()
{
  unsigned long fid = 0;

  if (m_sceneList->selectedItem() == NULL)
    {
      return NULL;
    }

  fid = static_cast<ListBoxIDItem*> (m_sceneList->selectedItem())->rtti();

  return static_cast<Scene*> (m_device->searchFunction(fid));
}

void SceneEditor::fillFunctions()
{
  QList <Function> *fl = m_device->functions();

  m_sceneList->clear();

  for (Function* f = fl->first(); f != NULL; f = fl->next())
    {
      if (f->type() == Function::Scene)
	{
	  ListBoxIDItem* item = new ListBoxIDItem();
	  item->setText(f->name());
	  item->setRtti(f->id());
	  m_sceneList->insertItem(item);
	}
    }

  m_sceneList->sort();

  setStatusText(KStatusUnchanged, KStatusColorUnchanged);
}

void SceneEditor::selectFunction(unsigned long fid)
{
  for (unsigned int i = 0; i < m_sceneList->count(); i++)
    {
      ListBoxIDItem* item = static_cast<ListBoxIDItem*> (m_sceneList->item(i));
      if (static_cast<unsigned int> (item->rtti()) == fid)
	{
	  m_sceneList->setSelected(item, true);
	  m_sceneList->ensureCurrentVisible();
	  break;
	}
    }
}


void SceneEditor::setStatusText(QString text, QColor color)
{
  m_statusLabel->setPaletteForegroundColor(color);
  m_statusLabel->setText(text);
}
