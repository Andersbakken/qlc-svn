/*
  Q Light Controller
  assignhotkey.cpp
  
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

#include "assignhotkey.h"
#include "keybind.h"
#include "uic_assignhotkey.h"
#include <qtextview.h>
#include <qevent.h>
#include <qlineedit.h>

AssignHotKey::AssignHotKey(QWidget* parent, const char* name)
  : UI_AssignHotKey(parent, name, true)
{
  m_keyBind = NULL;
  initView();
}

AssignHotKey::~AssignHotKey()
{
}

void AssignHotKey::initView()
{
  QString str = QString::null;
  str += QString("<HTML><HEAD><TITLE>Assign Key</TITLE></HEAD><BODY>");
  str += QString("<CENTER><H1>Assign Key</H1>");
  str += QString("Hit the key-combination that you want to assign to this widget. ");
  str += QString("You may hit either a single key or a modified key-combination");
  str += QString("that consists of CTRL, ALT, or SHIFT and a regular key.</CENTER>");
  str += QString("</BODY></HTML>");
  m_infoText->setText(str);
  m_infoText->setFocusPolicy(NoFocus);

  m_previewEdit->setReadOnly(true);
  m_previewEdit->setFocusPolicy(NoFocus);
  m_previewEdit->setAlignment(AlignCenter);

  connect((QObject*) m_ok, SIGNAL(clicked()), this, SLOT(slotOKClicked()));
  connect((QObject*) m_cancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));

}

void AssignHotKey::slotOKClicked()
{
  accept();
}

void AssignHotKey::slotCancelClicked()
{
  reject();
}

void AssignHotKey::keyPressEvent(QKeyEvent* e)
{
  if (m_keyBind != NULL)
    {
      delete m_keyBind;
      m_keyBind = NULL;
    }

  m_keyBind = new KeyBind(e);

  m_previewEdit->setText(m_keyBind->keyString());
}
