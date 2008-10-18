/*
  Q Light Controller
  inputchanneleditor.cpp

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

#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>

#include <common/qlcinputchannel.h>
#include "inputchanneleditor.h"

InputChannelEditor::InputChannelEditor(QWidget* parent,
				       QLCInputChannel* channel)
	: QDialog(parent)
{
	setupUi(this);

	/* Channel */
	Q_ASSERT(channel != NULL);
	m_channel = channel;

	/* Number */
	m_numberSpin->setValue(m_channel->channel());
	
	/* Name */
	m_nameEdit->setText(m_channel->name());

	/* Type */
	QString type;
	type = QLCInputChannel::typeToString(m_channel->type());
	m_typeCombo->setCurrentIndex(m_typeCombo->findText(type));
}

InputChannelEditor::~InputChannelEditor()
{
}

void InputChannelEditor::accept()
{
	/* Number */
	m_channel->setChannel(m_numberSpin->value());
	
	/* Name */
	m_channel->setName(m_nameEdit->text());

	/* Type */
	QLCInputChannel::Type type;
	type = QLCInputChannel::stringToType(m_typeCombo->currentText());
	m_channel->setType(type);
}
