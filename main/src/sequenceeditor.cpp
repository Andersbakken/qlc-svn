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
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpoint.h>
#include <qpointarray.h>

#include "patterngenerator.h"
#include "sequenceeditor.h"
#include "consolechannel.h"
#include "configkeys.h"
#include "settings.h"
#include "sequence.h"
#include "device.h"
#include "app.h"
#include "doc.h"

extern App* _app;

SequenceEditor::SequenceEditor(Sequence* sequence, QWidget* parent)
  : UI_SequenceEditor(parent),
    m_sequence   ( sequence ),
    m_channels   (        0 ),
    m_tempValues (     NULL )
{
}

SequenceEditor::~SequenceEditor()
{
  while (!m_unitList.isEmpty())
    {
      delete m_unitList.take(0);
    }

  delete [] m_tempValues;
}

void SequenceEditor::init()
{
  // Set an icon
  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH + QString("/");
  setIcon(QPixmap(dir + QString("chaser.xpm")));

  m_insert->setPixmap(QPixmap(dir + "/add.xpm"));
  m_remove->setPixmap(QPixmap(dir + "/remove.xpm"));

  m_raise->setPixmap(QPixmap(dir + "/up.xpm"));
  m_lower->setPixmap(QPixmap(dir + "/down.xpm"));

  m_list->setVScrollBarMode(QScrollView::AlwaysOn);
  m_list->header()->setClickEnabled(false);
  m_list->header()->setMovingEnabled(false);
  m_list->header()->setResizeEnabled(false);

  setDevice(m_sequence->device());
  setSequence(m_sequence);

  m_name->setText(m_sequence->name());
}

void SequenceEditor::setDevice(t_device_id id)
{
  Device* device = _app->doc()->device(id);
  assert(device);

  m_channels = device->deviceClass()->channels()->count();

  if (m_tempValues) delete [] m_tempValues;
  m_tempValues = new SceneValue[m_channels];

  QHBoxLayout* hbl = new QHBoxLayout(m_sliderContainer);

  ConsoleChannel* unit = NULL;
  for (t_channel ch = 0; ch < m_channels; ch++)
    {
      QString s;

      s.sprintf("%.3d", ch + 1);
      m_list->addColumn(s);

      unit = new ConsoleChannel(m_sliderContainer);
      unit->setDevice(id);
      unit->setChannel(ch);
      unit->setFadeStatusEnabled(false);
      m_unitList.append(unit);
      unit->update();
      
      // Channel updates to scene editor
      connect(unit, SIGNAL(changed(t_channel, t_value, Scene::ValueType)),
	      this, SLOT(slotChannelChanged(t_channel, t_value, 
					    Scene::ValueType)));

      connect(this, SIGNAL(sceneActivated(SceneValue*, t_channel)),
	      unit, SLOT(slotSceneActivated(SceneValue*, t_channel)));

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

void SequenceEditor::setSequence(Sequence* sequence)
{
  assert(sequence);
  m_sequence = sequence;

  QListViewItem* item = NULL;
  
  for (int i = m_sequence->m_values.count() - 1; i >= 0; i--)
    {
      QString s;
      item = new QListViewItem(m_list);
      for (t_channel ch = 0; ch < m_channels; ch++)
	{
	  s.sprintf("%.3d", m_sequence->m_values.at(i)[ch].value);
	  if (m_sequence->m_values.at(i)[ch].type == Scene::NoSet)
	    {
	      item->setText(ch, "XXX");
	    }
	  else
	    {
	      item->setText(ch, s);
	    }
	}
    }
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
  assert(channel <= m_channels);
  assert(m_tempValues);

  m_tempValues[channel].value = value;
  m_tempValues[channel].type = status;
  
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
  assert(m_tempValues);

  QListViewItem* item = NULL;
  if (m_list->currentItem())
    {
      item = new QListViewItem(m_list, m_list->currentItem());
    }
  else
    {
      item = new QListViewItem(m_list, m_list->lastItem());
    }

  QString s;
  for (t_channel ch = 0; ch < m_channels; ch++)
    {
      s.sprintf("%.3d", m_tempValues[ch].value);
      if (m_tempValues[ch].type == Scene::NoSet)
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
  
  // Set the item below selected
  if (m_list->currentItem())
    {
      m_list->setSelected(m_list->currentItem(), true);
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
  m_sequence->m_values.setAutoDelete(true);
  m_sequence->m_values.clear();
  m_sequence->m_values.setAutoDelete(false);
  
  for (QListViewItem* item = m_list->firstChild(); item != NULL;
       item = item->itemBelow())
    {
      SceneValue* value = new SceneValue[m_channels];
      for (t_channel i = 0; i < m_channels; i++)
	{
	  value[i].value = item->text(i).toInt();

	  if (item->text(i) != QString("XXX"))
	    {
	      value[i].type = Scene::Set;
	    }
	  else
	    {
	      value[i].type = Scene::NoSet;
	    }
	}

      m_sequence->m_values.append(value);
    }

  m_sequence->setName(m_name->text());

  accept();
}

void SequenceEditor::slotCancelClicked()
{
  reject();
}

void SequenceEditor::slotSelectionChanged(QListViewItem* item)
{
  if (item == NULL)
    {
      return;
    }

  for (t_channel i = 0; i < m_channels; i++)
    {
      if (item->text(i) == QString("XXX"))
	{
	  m_tempValues[i].value = 0;
	  m_tempValues[i].type = Scene::NoSet;
	}
      else
	{
	  m_tempValues[i].value = item->text(i).toInt();
	  m_tempValues[i].type = Scene::Set;
	}
    }

  emit sceneActivated(m_tempValues, m_channels);
}

void SequenceEditor::slotGeneratorButtonClicked()
{
  PatternGenerator* pg = new PatternGenerator(this);
  pg->setSequence(m_sequence);

  if (pg->exec() == QDialog::Accepted)
    {
      /*
      QPointArray a = pg->pointArray();
      for (int i = 0; i < a.size(); i++)
	{
	  QListViewItem* item = new QListViewItem(m_list);
	  QString s;
	  s.setNum(a.point(i).x());
	  item->setText(0, s);

	  s.setNum(a.point(i).y());
	  item->setText(1, s);
	}
      */
    }

  delete pg;
}
