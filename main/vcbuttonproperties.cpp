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

#include "common/qlcfixturedef.h"
#include "vcbuttonproperties.h"
#include "functionselection.h"
#include "virtualconsole.h"
#include "assignhotkey.h"
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

	/* press action */
	switch(m_keyBind->action())
	{
	default:
	case KeyBind::Toggle:
		m_toggle->setChecked(true);
		break;
	case KeyBind::Flash:
		m_flash->setChecked(true);
		break;
	case KeyBind::StepForward:
		m_forward->setChecked(true);
		break;
	case KeyBind::StepBackward:
		m_backward->setChecked(true);
		break;
	}

	/* Panic operation */
	m_stopFunctionsCheck->setChecked(m_button->stopFunctions());

	/* Button icons */
	m_attachFunction->setIcon(QIcon(":/attach.png"));
	m_detachFunction->setIcon(QIcon(":/detach.png"));
	m_attachKey->setIcon(QIcon(":/key_bindings.png"));
	m_detachKey->setIcon(QIcon(":/keyboard.png"));

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

void VCButtonProperties::accept()
{
	m_button->setCaption(m_nameEdit->text());
	m_button->setFunction(m_function);

	Q_ASSERT(m_keyBind != NULL);

	if (m_toggle->isChecked() == true)
		m_keyBind->setAction(KeyBind::Toggle);
	else if (m_flash->isChecked() == true)
		m_keyBind->setAction(KeyBind::Flash);
	else if (m_forward->isChecked() == true)
		m_keyBind->setAction(KeyBind::StepForward);
	else if (m_backward->isChecked() == true)
		m_keyBind->setAction(KeyBind::StepBackward);

	m_button->setKeyBind(m_keyBind);
        m_button->setStopFunctions(m_stopFunctionsCheck->isChecked());  

	_app->doc()->setModified();

	QDialog::accept();
}

