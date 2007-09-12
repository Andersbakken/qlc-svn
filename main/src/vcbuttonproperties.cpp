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

#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <stdlib.h>
#include <assert.h>
#include <qmessagebox.h>
#include <qcheckbox.h>

#include "common/qlcfixturedef.h"
#include "vcbuttonproperties.h"
#include "function.h"
#include "app.h"
#include "doc.h"
#include "assignhotkey.h"
#include "keybind.h"
#include "fixture.h"
#include "virtualconsole.h"
#include "functionmanager.h"

extern App* _app;

VCButtonProperties::VCButtonProperties(VCButton* button, QWidget* parent)
	: UI_VCButtonProperties(parent, "ButtonProperties", true),
	  
	  m_button     ( button ),
	  m_keyBind    ( new KeyBind(button->keyBind()) ),
	  m_functionID ( button->functionID() ),
	  m_functionManager ( NULL )
{
}


VCButtonProperties::~VCButtonProperties()
{
	delete m_keyBind;
}


void VCButtonProperties::initView()
{
	QString keyString;

	// Set name
	m_nameEdit->setText(m_button->text());

	// Set function name
	setFunctionName();

	// KeyBind key
	m_keyBind->keyString(keyString);
	m_keyEdit->setText(keyString);

	// Set press action
	m_onButtonPressGroup->setButton(m_keyBind->pressAction());
	slotPressGroupClicked(m_keyBind->pressAction());
	m_stopFunctionsCheck->setChecked(m_button->stopFunctions());
	//
	// Midi stuff
	//
	m_channelSpinBox->setValue(m_button->inputChannel());


	//
	// Pixmaps
	//
	m_attachFunction->setPixmap(QPixmap(QString(PIXMAPS) + QString("/attach.png")));
	m_detachFunction->setPixmap(QPixmap(QString(PIXMAPS) + QString("/detach.png")));

	m_attachKey->setPixmap(QPixmap(QString(PIXMAPS) + QString("/key_bindings.png")));
	m_detachKey->setPixmap(QPixmap(QString(PIXMAPS) + QString("/keyboard.png")));
}


void VCButtonProperties::setFunctionName()
{
	Fixture* fxi = NULL;
	Function* func = NULL;
	QString func_name;

	func = _app->doc()->function(m_functionID);
	if (func == NULL)
	{
		func_name = "No Function";
	}
	else
	{
		fxi = _app->doc()->fixture(func->fixture());
		if (fxi != NULL)
		{
			func_name = fxi->name() +
				QString(" / ") + func->name();
		}
		else
		{
			func_name = QString("Global") +
				QString(" / ") + func->name();
		}
	}

	m_functionEdit->setText(func_name);
}

void VCButtonProperties::slotPressGroupClicked(int id)
{
	ASSERT(m_keyBind != NULL);

	switch (id)
	{
	case 1:
		m_keyBind->setPressAction(KeyBind::PressToggle);
		m_keyBind->setReleaseAction(KeyBind::ReleaseNothing);
		break;

	case 2:
		m_keyBind->setPressAction(KeyBind::PressFlash);
		m_keyBind->setReleaseAction(KeyBind::ReleaseStop);
		break;

	case 3:
		m_keyBind->setPressAction(KeyBind::PressStepForward);
		m_keyBind->setReleaseAction(KeyBind::ReleaseNothing);
		break;

	case 4:
		m_keyBind->setPressAction(KeyBind::PressStepBackward);
		m_keyBind->setReleaseAction(KeyBind::ReleaseNothing);
		break;

	default:
		break;
	}
}

void VCButtonProperties::slotAttachFunctionClicked()
{
	m_functionManager = new FunctionManager(this,
						FunctionManager::SelectionMode);
	m_functionManager->init();

	connect(m_functionManager, SIGNAL(closed()),
		this, SLOT(slotFunctionManagerClosed()));

	m_functionManager->show();
}

void VCButtonProperties::slotFunctionManagerClosed()
{
	FunctionIDList list;

	assert(m_functionManager);

	if (m_functionManager->result() == QDialog::Accepted)
	{
		m_functionManager->selection(list);

		m_functionID = list.first();
		setFunctionName();

		list.clear();
	}

	delete m_functionManager;
	m_functionManager = NULL;
}


void VCButtonProperties::slotDetachFunctionClicked()
{
	m_functionID = KNoID;
	setFunctionName();
}

void VCButtonProperties::slotAttachKeyClicked()
{
	QString keyString;

	AssignHotKey* a = new AssignHotKey(this);
	if (a->exec() == QDialog::Accepted)
	{
		assert(a->keyBind());

		if (m_keyBind)
		{
			delete m_keyBind;
		}

		m_keyBind = new KeyBind(a->keyBind());

		m_keyBind->keyString(keyString);
		m_keyEdit->setText(keyString);
	}

	delete a;
}



void VCButtonProperties::slotDetachKeyClicked()
{
	QString keyString;

	m_keyBind->setKey(Key_unknown);
	m_keyBind->setMod(NoButton);

	m_keyBind->keyString(keyString);
	m_keyEdit->setText(keyString);
}



void VCButtonProperties::slotOKClicked()
{
	m_button->setCaption(m_nameEdit->text());
	m_button->attachFunction(m_functionID);
	m_button->setKeyBind(m_keyBind);
	m_button->setInputChannel(m_channelSpinBox->value());
        m_button->setStopFunctions(m_stopFunctionsCheck->isChecked());  

	_app->doc()->setModified();

	accept();
}

void VCButtonProperties::slotCancelClicked()
{
	reject();
}

