/*
  Q Light Controller
  vcbuttonproperties.cpp

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

#include <QRadioButton>
#include <QMessageBox>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>

#include "common/qlcinputchannel.h"
#include "common/qlcinputdevice.h"
#include "common/qlcfixturedef.h"

#include "vcbuttonproperties.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "virtualconsole.h"
#include "assignhotkey.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "function.h"
#include "keybind.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

VCButtonProperties::VCButtonProperties(VCButton* button, QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);

	/* Button text and function */
	m_button = button;
	m_nameEdit->setText(m_button->caption());
	slotSetFunction(button->function());

	/* KeyBind key */
	m_keyBind = new KeyBind(button->keyBind());
	m_keyEdit->setText(m_keyBind->keyString());

	/* External input */
	m_inputUniverse = m_button->inputUniverse();
	m_inputChannel = m_button->inputChannel();
	updateInputSource();

	connect(m_chooseInputButton, SIGNAL(clicked()),
		this, SLOT(slotChooseInputClicked()));

	/* Press action */
	switch(m_keyBind->action())
	{
	default:
	case KeyBind::Toggle:
		m_toggle->setChecked(true);
		break;
	case KeyBind::Flash:
		m_flash->setChecked(true);
		break;
	}

	/* Panic operation */
	m_stopFunctionsCheck->setChecked(m_button->stopFunctions());

	/* Button connections */
	connect(m_attachFunction, SIGNAL(clicked()),
		this, SLOT(slotAttachFunction()));
	connect(m_detachFunction, SIGNAL(clicked()),
		this, SLOT(slotSetFunction()));
	connect(m_attachKey, SIGNAL(clicked()), this, SLOT(slotAttachKey()));
	connect(m_detachKey, SIGNAL(clicked()), this, SLOT(slotDetachKey()));
}

VCButtonProperties::~VCButtonProperties()
{
	delete m_keyBind;
}

void VCButtonProperties::slotAttachFunction()
{
	FunctionSelection sel(this, false, KNoID, Function::Undefined, false);
	if (sel.exec() == QDialog::Accepted)
	{
		/* Get the first selected function */
		slotSetFunction(sel.selection.at(0));
	}
}

void VCButtonProperties::slotSetFunction(t_function_id fid)
{
	Function* func;

	m_function = fid;

	func = _app->doc()->function(m_function);
	if (func == NULL)
		m_functionEdit->setText("No function");
	else
		m_functionEdit->setText(func->name());
}

void VCButtonProperties::slotAttachKey()
{
	AssignHotKey ahk(this);
	if (ahk.exec() == QDialog::Accepted)
	{
		QString keyString;

		Q_ASSERT(ahk.keyBind() != NULL);

		if (m_keyBind != NULL)
			delete m_keyBind;
		m_keyBind = new KeyBind(ahk.keyBind());
		m_keyEdit->setText(m_keyBind->keyString());
	}
}

void VCButtonProperties::slotDetachKey()
{
	m_keyBind->setKey(Qt::Key_unknown);
	m_keyBind->setMod(Qt::NoModifier);
	m_keyEdit->setText(m_keyBind->keyString());
}

void VCButtonProperties::slotChooseInputClicked()
{
        SelectInputChannel sic(this);
        if (sic.exec() == QDialog::Accepted)
        {
                m_inputUniverse = sic.universe();
                m_inputChannel = sic.channel();

                updateInputSource();
        }
}

void VCButtonProperties::updateInputSource()
{
	QLCInputDevice* dev;
	InputPatch* patch;
	QString uniName;
	QString chName;

	if (m_inputUniverse == KInputUniverseInvalid ||
	    m_inputChannel == KInputChannelInvalid)
	{
		/* Nothing selected for input universe and/or channel */
		uniName = KInputNone;
		chName = KInputNone;
	}
	else
	{
		patch = _app->inputMap()->patch(m_inputUniverse);
		if (patch == NULL || patch->plugin() == NULL)
		{
			/* There is no patch for the given universe */
			uniName = KInputNone;
			chName = KInputNone;
		}
		else
		{
			dev = patch->deviceTemplate();
			if (dev == NULL)
			{
				/* There is no device template. Display plugin
				   name and channel number. Boring. */
				uniName = patch->plugin()->name();
				chName = tr("%1: Unknown").arg(m_inputChannel);
			}
			else
			{
				/* Display template name for universe */
				uniName = QString("%1: %2")
						.arg(m_inputUniverse + 1)
						.arg(dev->name());

				/* User can input the channel number by hand,
				   so put something rational to the channel
				   name in those cases as well. */
				QString name = dev->channelName(m_inputChannel);
				if (name == QString::null)
					name = tr("Unknown");

				/* Display channel name */
				chName = QString("%1: %2")
					.arg(m_inputChannel + 1).arg(name);
			}
		}
	}

	/* Display the gathered information */
	m_inputUniverseEdit->setText(uniName);
	m_inputChannelEdit->setText(chName);
}

void VCButtonProperties::accept()
{
	m_button->setCaption(m_nameEdit->text());
	m_button->setFunction(m_function);

	Q_ASSERT(m_keyBind != NULL);

	if (m_flash->isChecked() == true)
		m_keyBind->setAction(KeyBind::Flash);
	else
		m_keyBind->setAction(KeyBind::Toggle);

	m_button->setKeyBind(m_keyBind);
	m_button->setInputSource(m_inputUniverse, m_inputChannel);
        m_button->setStopFunctions(m_stopFunctionsCheck->isChecked());

	_app->doc()->setModified();

	QDialog::accept();
}

