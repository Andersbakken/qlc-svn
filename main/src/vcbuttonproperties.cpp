/*
  Q Light Controller
  vcbuttonproperties.cpp
  
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

#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <stdlib.h>

#include "vcbuttonproperties.h"
#include "deviceclass.h"
#include "function.h"
#include "functiontree.h"
#include "settings.h"
#include "app.h"
#include "doc.h"
#include "assignhotkey.h"
#include "keybind.h"
#include "devicemanagerview.h"
#include "devicemanager.h"
#include "device.h"
#include "virtualconsole.h"
#include "configkeys.h"

extern App* _app;

VCButtonProperties::VCButtonProperties(VCButton* btn, QWidget* parent,
                                       const char* name) 
  : UI_VCButtonProperties(parent, name, true)
{
  ASSERT(btn != NULL);
  m_button = btn;

  m_function = btn->function();
  m_lockState = btn->locked();
  m_bus = btn->speedBus();

  m_busIndex = NULL;

  ASSERT (btn->keyBind() != NULL);
  m_keyBind = new KeyBind((KeyBind*) btn->keyBind());

  initView();
}

VCButtonProperties::~VCButtonProperties()
{
  if (m_busIndex != NULL)
    {
      delete m_busIndex;
    }
}

void VCButtonProperties::initView()
{
  QString fname;

  m_nameEdit->setText( ((VCWidgetBase*) m_button)->name());

  if (m_function == NULL)
    {
      fname = "No Function";
    }
  else
    {
      if (m_function->device())
	{
	  fname = m_function->device()->name();
	}
      else
	{
	  fname = QString("Global/");
	}

      fname += QString("/") + m_function->name();
    }

  m_functionEdit->setReadOnly(true);
  m_functionEdit->setText(fname);

  // Bus stuff
  fillBusCombo();

  if (m_busCombo->count() != 1)
    {
      // Select current bus if present
      if (m_bus != NULL)
	{
	  selectCurrentBus();
	}
      else
	{
	  m_busCombo->setCurrentItem(0);
	}
    }
  else
    {
      m_busCombo->setCurrentItem(0);
    }

  // Widget position lock
  m_lock->setOn(m_lockState);

  // KeyBind key
  m_keyEdit->setReadOnly(true);
  m_keyEdit->setFocusPolicy(NoFocus);
  m_keyEdit->setAlignment(AlignCenter);
  m_keyEdit->setText(m_keyBind->keyString());

  // Action groups
  m_onButtonPressGroup->setEnabled(true);
  m_onButtonReleaseGroup->setEnabled(true);

  m_onButtonPressGroup->setButton(m_keyBind->pressAction());
  m_onButtonReleaseGroup->setButton(m_keyBind->releaseAction());

  // This disables the release group if action group wants that
  slotPressGroupClicked(m_keyBind->pressAction());

  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

  // Pixmaps
  m_addFunction->setPixmap(QPixmap(dir + "/attach.xpm"));
  m_removeFunction->setPixmap(QPixmap(dir + "/detach.xpm"));

  m_addKey->setPixmap(QPixmap(dir + "/key.xpm"));
  m_removeKey->setPixmap(QPixmap(dir + "/fileclose.xpm"));

  m_addBus->setPixmap(QPixmap(dir + "/addbus.xpm"));

  // Connections
  connect((QObject*) m_onButtonPressGroup, SIGNAL(clicked(int)), this, SLOT(slotPressGroupClicked(int)));
  connect((QObject*) m_onButtonReleaseGroup, SIGNAL(clicked(int)), this, SLOT(slotReleaseGroupClicked(int)));

  connect((QObject*) m_addFunction, SIGNAL(clicked()), this, SLOT(slotAddFunctionClicked()));
  connect((QObject*) m_removeFunction, SIGNAL(clicked()), this, SLOT(slotRemoveFunctionClicked()));

  connect((QObject*) m_addKey, SIGNAL(clicked()), this, SLOT(slotAddKeyClicked()));
  connect((QObject*) m_removeKey, SIGNAL(clicked()), this, SLOT(slotRemoveKeyClicked()));

  connect((QObject*) m_addBus, SIGNAL(clicked()), this, SLOT(slotAddBusClicked()));
  connect((QObject*) m_busCombo, SIGNAL(activated(int)), this, SLOT(slotBusActivated(int)));

  connect((QObject*) m_ok, SIGNAL(clicked()), this, SLOT(slotOKClicked()));
  connect((QObject*) m_cancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
}

void VCButtonProperties::slotOKClicked()
{
  m_nameString = m_nameEdit->text();
  m_lockState = m_lock->isOn();

  _app->doc()->setModified(true);

  accept();
}

void VCButtonProperties::slotCancelClicked()
{
  reject();
}

void VCButtonProperties::slotPressGroupClicked(int id)
{
  ASSERT(m_keyBind != NULL);

  switch (id)
    {
    case 0:
      m_onButtonReleaseGroup->setEnabled(true);
      m_keyBind->setPressAction(KeyBind::PressStart);
      break;

    case 1:
      m_onButtonReleaseGroup->setButton(1);
      m_onButtonReleaseGroup->setEnabled(false);
      m_keyBind->setPressAction(KeyBind::PressToggle);
      m_keyBind->setReleaseAction(KeyBind::ReleaseNothing);
      break;

    case 2:
      m_onButtonReleaseGroup->setEnabled(false);
      m_keyBind->setPressAction(KeyBind::PressStepForward);
      break;

    case 3:
      m_onButtonReleaseGroup->setEnabled(false);
      m_keyBind->setPressAction(KeyBind::PressStepBackward);
      break;

    case 4:
      m_onButtonReleaseGroup->setButton(1);
      m_onButtonReleaseGroup->setEnabled(false);
      m_keyBind->setPressAction(KeyBind::PressStop);
      m_keyBind->setReleaseAction(KeyBind::ReleaseNothing);
      break;

    case 5:
      m_onButtonReleaseGroup->setEnabled(true);
      m_keyBind->setPressAction(KeyBind::PressNothing);
      break;

    default:
      break;
    }
}

void VCButtonProperties::slotReleaseGroupClicked(int id)
{
  ASSERT(m_keyBind != NULL);

  switch (id)
    {
    case 0:
      m_keyBind->setReleaseAction(KeyBind::ReleaseStop);
      break;

    case 1:
      m_keyBind->setReleaseAction(KeyBind::ReleaseNothing);
      break;

    default:
      break;
    }
}

void VCButtonProperties::slotAddFunctionClicked()
{
  QString str("No function");

  FunctionTree* ft = new FunctionTree(this);
  if (ft->exec() == QDialog::Accepted)
    {
      if (m_function)
	{
	  //
	  // Disconnect destroyed signal from previous function
	  // disconnect(m_function, SIGNAL(destroyed()), this, SLOT(slotFunctionDestroyed()));
	}
      
      //
      // Get the function that was selected
      m_function = _app->doc()->searchFunction(ft->functionId());

      if (m_function)
	{
	  //
	  // Connect destroyed signal to new function
	  // connect(m_function, SIGNAL(destroyed()), this, SLOT(slotFunctionDestroyed()));
	  
	  if (m_function->device())
	    {
	      str = m_function->device()->name();
	    }
	  else
	    {
	      str = QString("Global");
	    }
	  
	  str += QString("/") + m_function->name();
	}
    }

  //
  // Set the correct text to edit box
  m_functionEdit->setText(str);

}

void VCButtonProperties::slotRemoveFunctionClicked()
{
  m_function = NULL;

  m_functionEdit->setText("No Function");
}

void VCButtonProperties::slotAddKeyClicked()
{
  AssignHotKey* a = NULL;
  a = new AssignHotKey(this);

  if (a->exec() == QDialog::Accepted)
    {
      ASSERT (m_keyBind != NULL);
      ASSERT (a->keyBind() != NULL);

      m_keyBind->setKey(a->keyBind()->key(), a->keyBind()->mod());
      m_keyEdit->setText(m_keyBind->keyString());
    }

  delete a;
}

void VCButtonProperties::slotRemoveKeyClicked()
{
  m_keyBind->setKey(0, 0);
  m_keyEdit->setText(m_keyBind->keyString());
}

void VCButtonProperties::slotBusActivated(int item)
{
  if (item != 0)
    {
      t_bus_id busIndex = m_busIndex[m_busCombo->currentItem() - 1];
      m_bus = _app->doc()->searchBus(busIndex);
      ASSERT(m_bus != NULL);
      qDebug("Activated bus " + m_bus->name());
    }
  else
    {
      m_bus = NULL;
    }
}

void VCButtonProperties::fillBusCombo()
{
  Bus* bus = NULL;
  unsigned int i = 0;
  unsigned int j = 0;

  m_busCombo->clear();
  m_busCombo->insertItem(QString("None"));

  if (m_busIndex != NULL)
    {
      delete m_busIndex;
    }

  m_busIndex = new t_bus_id [_app->doc()->busList()->count()];

  for (i = 0; i < _app->doc()->busList()->count(); i++)
    {
      bus = _app->doc()->busList()->at(i);

      m_busCombo->insertItem(bus->name());
      m_busIndex[j++] = bus->id();
    }
}

void VCButtonProperties::selectCurrentBus()
{
  ASSERT(m_bus != NULL);
  ASSERT(m_busIndex != NULL);

  // Set "None" as the current item by default
  m_busCombo->setCurrentItem(0);

  for (t_bus_id i = 0; i < (t_bus_id) _app->doc()->busList()->count(); i++)
    {
      if (m_bus->id() == m_busIndex[i])
	{
	  m_busCombo->setCurrentItem(i+1);
	  break;
	}
    }
}

void VCButtonProperties::slotAddBusClicked()
{
  Bus* bus = _app->deviceManagerView()->deviceManager()->slotDLAddBus(this);

  if (bus != NULL)
    {
      fillBusCombo();
      m_bus = bus;
      _app->virtualConsole()->setDefaultSpeedBus(m_bus);
      selectCurrentBus();
    }
}
