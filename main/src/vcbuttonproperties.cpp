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
#include <assert.h>

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
#include "device.h"
#include "virtualconsole.h"
#include "configkeys.h"

extern App* _app;

VCButtonProperties::VCButtonProperties(VCButton* button, QWidget* parent,
                                       const char* name) 
  : UI_VCButtonProperties(parent, name, true),

    m_button     ( button               ),
    m_keyBind    ( button->keyBind()    ),
    m_functionID ( button->functionID() )
{
  initView();
}


VCButtonProperties::~VCButtonProperties()
{
}


void VCButtonProperties::initView()
{
  // Set name
  m_nameEdit->setText(m_button->text());

  // Set function name
  setFunctionName();

  // KeyBind key
  m_keyEdit->setText(m_keyBind->keyString());

  // Set press action
  m_onButtonPressGroup->setButton(m_keyBind->pressAction());
  slotPressGroupClicked(m_keyBind->pressAction());

  //
  // Pixmaps
  //
  m_attachFunction->setPixmap(QPixmap(QString(PIXMAPS) + QString("/attach.xpm")));
  m_detachFunction->setPixmap(QPixmap(QString(PIXMAPS) + QString("/detach.xpm")));

  m_attachKey->setPixmap(QPixmap(QString(PIXMAPS) + QString("/key.xpm")));
  m_detachKey->setPixmap(QPixmap(QString(PIXMAPS) + QString("/fileclose.xpm")));
}


void VCButtonProperties::setFunctionName()
{
  QString fname;

  Function* f = _app->doc()->function(m_functionID);
  if (f == NULL)
    {
      fname = "No Function";
    }
  else
    {
      Device* d = _app->doc()->device(f->device());
      if (d)
	{
	  fname = d->name() + QString(" / ") + f->name();
	}
      else
	{
	  fname = QString("Global") + QString(" / ") + f->name();
	}
    }

  m_functionEdit->setText(fname);
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
  FunctionTree* ft = new FunctionTree(this);
  if (ft->exec() == QDialog::Accepted)
    {
      m_functionID = ft->functionID();
      setFunctionName();
    }

  delete ft;
}

void VCButtonProperties::slotDetachFunctionClicked()
{
  m_functionID = KNoID;
  setFunctionName();
}

void VCButtonProperties::slotAttachKeyClicked()
{
  AssignHotKey* a = NULL;
  a = new AssignHotKey(this);

  if (a->exec() == QDialog::Accepted)
    {
      assert(m_keyBind);
      assert(a->keyBind());

      m_keyBind->setKey(a->keyBind()->key());
      m_keyBind->setMod(a->keyBind()->mod());
      m_keyEdit->setText(m_keyBind->keyString());
    }

  delete a;
}

void VCButtonProperties::slotDetachKeyClicked()
{
  m_keyBind->setKey(0);
  m_keyBind->setMod(NoButton);
  m_keyEdit->setText(m_keyBind->keyString());
}

void VCButtonProperties::slotOKClicked()
{
  m_button->setCaption(m_nameEdit->text());
  m_button->attachFunction(m_functionID);
  m_button->setKeyBind(m_keyBind);

  _app->doc()->setModified(true);

  accept();
}

void VCButtonProperties::slotCancelClicked()
{
  reject();
}

