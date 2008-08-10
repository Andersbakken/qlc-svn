/*
  Q Light Controller
  editscenevalue.cpp

  Copyright (c) Heikki Junnila

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

#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QList>

#include "common/qlcchannel.h"
#include "common/qlccapability.h"
#include "editscenevalue.h"

EditSceneValue::EditSceneValue(QWidget* parent, QLCChannel* ch,
			       SceneValue &currentValue) : QDialog(parent)
{
	Q_ASSERT(ch != NULL);

	m_channel = ch;
	m_updateValue = true;

	setupUi(this);

	QListIterator <QLCCapability*> it(*m_channel->capabilities());
	while (it.hasNext() == true)
		m_presetCombo->addItem(it.next()->name());

	m_typeCombo->addItem("Fade");
	m_typeCombo->addItem("Set");
	m_typeCombo->addItem("NoSet");

	if (currentValue.type == Scene::Fade)
	{
		m_typeCombo->setCurrentIndex(0);
		m_type = QString("Fade");
	}
	else if (currentValue.type == Scene::Set)
	{
		m_typeCombo->setCurrentIndex(1);
		m_type = QString("Set");
	}
	else
	{
		m_typeCombo->setCurrentIndex(2);
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
	QLCCapability* cap;
	int index;

	if (m_updateValue == false)
		return;
	m_updateValue = false;

	cap = m_channel->searchCapability(value);
	if (cap == NULL)
	{
		/* Capability by the entered DMX value was not found. Select
		   the first capability as fallback... */
		m_presetCombo->setCurrentIndex(0);
	}
	else
	{
		/* Select a capability that matches the entered DMX value. */
		index = m_presetCombo->findText(cap->name());
		m_presetCombo->setCurrentIndex(index);
	}

	m_value = value;
	m_updateValue = true;
}

void EditSceneValue::slotPresetComboActivated(const QString &text)
{
	QLCCapability* cap;

	if (m_updateValue == false)
		return;
	m_updateValue = false;

	cap = m_channel->searchCapability(text);
	if (cap == NULL)
	{
		/* Selected capability not found. Weird. Set value to zero. */
		m_valueSpin->setValue(0);
	}
	else
	{
		/* Set the DMX value from the middle of the capability's
		   minimum and maximum values. */
		t_value value = (t_value) floor((cap->min() + cap->max()) / 2);
		m_valueSpin->setValue(value);
		m_value = value;
	}

	m_updateValue = true;
}

void EditSceneValue::slotTypeActivated(const QString &text)
{
	m_type = text;
}
