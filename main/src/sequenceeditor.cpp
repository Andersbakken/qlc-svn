/*
  Q Light Controller
  sequenceeditor.cpp
  
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

#include <assert.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qheader.h>
#include <qscrollview.h>
#include <qscrollbar.h>

#include "sequenceeditor.h"
#include "consolechannel.h"
#include "configkeys.h"
#include "settings.h"
#include "device.h"
#include "app.h"
#include "doc.h"

extern App* _app;

SequenceEditor::SequenceEditor(QWidget* parent, const char* name)
  : UI_SequenceEditor(parent, name),
    m_tempScene ( NULL ),
    m_channels  (    0 )
{
}

SequenceEditor::~SequenceEditor()
{
  while (!m_unitList.isEmpty())
    {
      delete m_unitList.take(0);
    }

  if (m_tempScene) delete m_tempScene;
}

void SequenceEditor::init()
{
  // Set an icon
  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH + QString("/");
  //setIcon(QPixmap(dir + QString("sequence.xpm")));

  m_insert->setPixmap(QPixmap(dir + "/add.xpm"));
  m_remove->setPixmap(QPixmap(dir + "/remove.xpm"));

  m_raise->setPixmap(QPixmap(dir + "/up.xpm"));
  m_lower->setPixmap(QPixmap(dir + "/down.xpm"));

  m_list->setVScrollBarMode(QScrollView::AlwaysOn);
  m_list->header()->setClickEnabled(false);
  m_list->header()->setMovingEnabled(false);
  m_list->header()->setResizeEnabled(false);
}

void SequenceEditor::setDevice(t_device_id id)
{
  Device* device = _app->doc()->device(id);
  assert(device);

  if (m_tempScene) delete m_tempScene;
  m_tempScene = new Scene();
  m_tempScene->setDevice(id);

  QHBoxLayout* hbl = new QHBoxLayout(m_sliderContainer);

  QString s;
  m_channels = device->deviceClass()->channels()->count();

  ConsoleChannel* unit = NULL;
  for (t_channel ch = 0; ch < m_channels; ch++)
    {
      s.sprintf("%.3d", ch + 1);
      m_list->addColumn(s);

      unit = new ConsoleChannel(m_sliderContainer);
      unit->setDevice(id);
      unit->setChannel(ch);
      m_unitList.append(unit);
      unit->update();
      
      // Channel updates to scene editor
      connect(unit, SIGNAL(changed(t_channel, t_value, Scene::ValueType)),
	      this, SLOT(slotChannelChanged(t_channel, t_value, 
					    Scene::ValueType)));

      // Add the unit to the layout
      hbl->addWidget(unit);
      
      // Add a spacer between all consolechannel units
      // QT deletes spacers automagically
      QSpacerItem* sp = new QSpacerItem(m_list->header()->sectionSize(ch) 
					- 35, 0, QSizePolicy::Minimum);
      hbl->addItem(sp);
      m_spacerList.append(sp);
      
    }

  // Add a spacer that has the size of the listview's scrollbar
  QSpacerItem* sp = new QSpacerItem(m_list->verticalScrollBar()->width(), 
				    0, QSizePolicy::Fixed);
  hbl->addItem(sp);

  m_list->setResizeMode(QListView::AllColumns);
  m_list->setSorting(-1);
}

void SequenceEditor::resizeEvent(QResizeEvent* e)
{
  for (t_channel i = 0; i < m_channels; i++)
    {
      // Resize the spacers between consolechannel units to match
      // (well, more or less) the header size in the listview
      int s = m_list->header()->sectionSize(i) - 35;
      m_spacerList.at(i)->changeSize(s, 0);
    }

  UI_SequenceEditor::resizeEvent(e);
}

void SequenceEditor::slotChannelChanged(t_channel channel, t_value value, 
					Scene::ValueType status)
{
  m_tempScene->set(channel, value, status);
  
  if (m_list->currentItem())
    {
      QString s;
      s.sprintf("%.3d", value);
      if (status == Scene::NoSet)
	{
	  m_list->currentItem()->setText(channel, "XXX");
	}
      else
	{
	  m_list->currentItem()->setText(channel, s);
	}
    }
}

void SequenceEditor::slotInsert()
{
  assert(m_tempScene);

  QListViewItem* item = NULL;
  if (m_list->currentItem())
    {
      item = new QListViewItem(m_list, m_list->currentItem());
    }
  else
    {
      item = new QListViewItem(m_list, m_list->lastItem());
    }
  m_list->setSelected(item, true);

  QString s;
  for (t_channel ch = 0; ch < m_channels; ch++)
    {
      s.sprintf("%.3d", m_tempScene->channelValue(ch).value);
      if (m_tempScene->channelValue(ch).type == Scene::NoSet)
	{
	  item->setText(ch, "XXX");
	}
      else
	{
	  item->setText(ch, s);
	}
    }

  m_list->setSelected(item, true);
  m_list->ensureItemVisible(item);
}

void SequenceEditor::slotRemove()
{
  if (m_list->currentItem())
    {
      delete m_list->currentItem();
    }
}

void SequenceEditor::slotRaise()
{
  QListViewItem* item = m_list->currentItem();
  if (!item)
    {
      return;
    }

  QListViewItem* itemAbove = item->itemAbove();
  if (itemAbove)
    {
      if (itemAbove->itemAbove())
	{
	  item->moveItem(itemAbove->itemAbove());
	}
      else
	{
	  itemAbove->moveItem(item);
	}
    }

  m_list->ensureItemVisible(item);
}

void SequenceEditor::slotLower()
{
  QListViewItem* item = m_list->currentItem();
  if (!item)
    {
      return;
    }

  QListViewItem* itemBelow = item->itemBelow();
  if (itemBelow)
    {
      item->moveItem(itemBelow);
    }

  m_list->ensureItemVisible(item);
}

void SequenceEditor::slotOKClicked()
{
  accept();
}

void SequenceEditor::slotCancelClicked()
{
  reject();
}
