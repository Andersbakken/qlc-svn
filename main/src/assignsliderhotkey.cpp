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

#include "assignsliderhotkey.h"
#include "sliderkeybind.h"
#include "uic_assignsliderhotkey.h"
#include <qtextview.h>
#include <qevent.h>
#include <qlineedit.h>

AssignSliderHotKey::AssignSliderHotKey(QWidget* parent, const char* name)
  : UI_AssignSliderHotKey(parent, name, true)
{
  m_sliderKeyBind = new SliderKeyBind();
  initView();
}

AssignSliderHotKey::~AssignSliderHotKey()
{
}

void AssignSliderHotKey::initView()
{
  QString str = QString::null;
  str += QString("<HTML><HEAD><TITLE>Assign Slider Keys</TITLE></HEAD><BODY>");
  str += QString("<CENTER><H1>Assign Slider Keys</H1>");
  str += QString("Move the Mouse Cursor over the field, for the orientation you want to assign. ");
  str += QString("Hit the key-combination that you want to assign. ");
  str += QString("You may hit either a single key or a key-combination");
  str += QString("with a CTRL, ALT, and/or SHIFT.</CENTER>");
  str += QString("</BODY></HTML>");
  m_infoText->setText(str);
  m_infoText->setFocusPolicy(NoFocus);

  m_previewUpEdit->setReadOnly(true);
  m_previewUpEdit->setFocusPolicy(NoFocus);
  m_previewUpEdit->setAlignment(AlignCenter);

  m_previewDownEdit->setReadOnly(true);
  m_previewDownEdit->setFocusPolicy(NoFocus);
  m_previewDownEdit->setAlignment(AlignCenter);

  connect((QObject*) m_ok, SIGNAL(clicked()),
	  this, SLOT(slotOKClicked()));
  connect((QObject*) m_cancel, SIGNAL(clicked()),
	  this, SLOT(slotCancelClicked()));

}

void AssignSliderHotKey::slotOKClicked()
{
  accept();
}

void AssignSliderHotKey::slotCancelClicked()
{
  reject();
}

void AssignSliderHotKey::keyPressEvent(QKeyEvent* e)
{
  if (m_previewUpEdit->hasMouse())
    {
      QString keyStringUp;

      ASSERT (m_sliderKeyBind != NULL);

      m_sliderKeyBind->setKeyUp(e->key());
      m_sliderKeyBind->setModUp(e->state());

      m_sliderKeyBind->keyStringUp(keyStringUp);
      m_previewUpEdit->setText(keyStringUp);
    }
  else if (m_previewDownEdit->hasMouse())
    {
      QString keyStringDown;

      ASSERT (m_sliderKeyBind != NULL);

      m_sliderKeyBind->setKeyDown(e->key());
      m_sliderKeyBind->setModDown(e->state());

      m_sliderKeyBind->keyStringDown(keyStringDown);
      m_previewDownEdit->setText(keyStringDown);
    }
}
