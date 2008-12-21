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
#include <QIcon>

#include <common/qlcinputchannel.h>
#include "inputchanneleditor.h"

InputChannelEditor::InputChannelEditor(QWidget* parent,
	const QLCInputChannel* channel) : QDialog(parent)
{
	m_channel = KInputChannelInvalid;
	m_type = QLCInputChannel::NoType;
	m_name = QString::null;

	setupUi(this);

	/* Fill type combo with type icons and names */
	QStringListIterator it(QLCInputChannel::types());
	while (it.hasNext() == true)
	{
		QString str(it.next());
		if (str == KXMLQLCInputChannelButton)
			m_typeCombo->addItem(QIcon(":/button.png"), str);
		else if (str == KXMLQLCInputChannelKnob)
			m_typeCombo->addItem(QIcon(":/knob.png"), str);
		else if (str == KXMLQLCInputChannelSlider)
			m_typeCombo->addItem(QIcon(":/slider.png"), str);
	}

	if (channel != NULL)
	{
		QString type;

		/* Channel number */
		m_numberSpin->setValue(channel->channel() + 1);

		/* Channel name */
		m_nameEdit->setText(channel->name());

		/* Channel type */
		type = QLCInputChannel::typeToString(channel->type());
		m_typeCombo->setCurrentIndex(m_typeCombo->findText(type));
	}
	else
	{
		/* Multiple channels are being edited */
		m_numberSpin->setEnabled(false);
	}

	connect(m_numberSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotNumberChanged(int)));
	connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
		this, SLOT(slotNameEdited(const QString&)));
	connect(m_typeCombo, SIGNAL(activated(const QString&)),
		this, SLOT(slotTypeActivated(const QString &)));
}

InputChannelEditor::~InputChannelEditor()
{
}

void InputChannelEditor::slotNumberChanged(int number)
{
	m_channel = number - 1;
}

void InputChannelEditor::slotNameEdited(const QString& text)
{
	m_name = text;
}

void InputChannelEditor::slotTypeActivated(const QString& text)
{
	m_type = QLCInputChannel::stringToType(text);
}
