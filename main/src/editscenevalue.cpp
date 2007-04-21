/*
  Q Light Controller
  editscenevalue.cpp

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

#include <qwidget.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qptrlist.h>
#include <math.h>

#include "common/logicalchannel.h"
#include "common/capability.h"
#include "editscenevalue.h"

EditSceneValue::EditSceneValue(QWidget* parent, LogicalChannel* ch,
			       SceneValue &currentValue)
  : UI_EditSceneValue(parent, "", true)
{
  ASSERT(ch != NULL);

  m_channel = ch;
  m_updateValue = true;

  for (Capability* c = m_channel->capabilities()->first(); 
       c != NULL; c = m_channel->capabilities()->next())
    {
      m_presetCombo->insertItem(c->name());
    }

  m_typeCombo->insertItem("Fade");
  m_typeCombo->insertItem("Set");
  m_typeCombo->insertItem("NoSet");

  if (currentValue.type == Scene::Fade)
    {
      m_typeCombo->setCurrentItem(0);
      m_type = QString("Fade");
    }
  else if (currentValue.type == Scene::Set)
    {
      m_typeCombo->setCurrentItem(1);
      m_type = QString("Set");
    }
  else
    {
      m_typeCombo->setCurrentItem(2);
      m_type = QString("NoSet");
    }

  connect(m_valueSpin, SIGNAL(valueChanged(int)), 
	  this, SLOT(slotValueChanged(int)));
  connect(m_presetCombo, SIGNAL(activated(const QString &)), 
	  this, SLOT(slotPresetComboActivated(const QString &)));

  m_valueSpin->setValue(currentValue.value);
  m_value = currentValue.value;
}

EditSceneValue::~EditSceneValue()
{
}

void EditSceneValue::slotValueChanged(int value)
{
  if (m_updateValue == false)
    {
      return;
    }

  m_updateValue = false;

  Capability* c = m_channel->searchCapability(value);
  ASSERT(c != NULL);

  for (int i = 0; i < m_presetCombo->count(); i++)
    {
      if (m_presetCombo->text(i) == c->name())
	{
	  m_presetCombo->setCurrentItem(i);
	  break;
	}
    }

  m_value = value;

  m_updateValue = true;
}

void EditSceneValue::slotPresetComboActivated(const QString &text)
{
  if (m_updateValue == false)
    {
      return;
    }

  m_updateValue = false;

  Capability* c = m_channel->searchCapability(text);
  ASSERT(c != NULL);

  int value = (int) floor((c->lo() + c->hi()) / 2);
  m_valueSpin->setValue(value);
  m_value = value;

  m_updateValue = true;
}

void EditSceneValue::slotTypeActivated(const QString &text)
{
  m_type = text;
}
