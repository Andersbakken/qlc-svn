/*
  Q Light Controller
  aboutbox.cpp
  
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

#include "aboutbox.h"
#include <qpixmap.h>
#include <qcolor.h>

AboutBox::AboutBox(QWidget* parent, const char* name) : QDialog (parent, name, true)
{
  initDialog();
}

AboutBox::~AboutBox()
{

}

void AboutBox::initDialog()
{
  setCaption("About QLC");
  resize(200, 200);
  setMinimumSize(200, 200);
  setMaximumSize(200, 200);

  m_qlc = new QLabel(this);
  m_qlc->setGeometry(10, 10, 180, 20);
  m_qlc->setAlignment(AlignCenter);
  m_qlc->setText("Q Light Controller 2.0.10");
  
  m_copyright = new QLabel(this);
  m_copyright->setGeometry(10, 30, 180, 20);
  m_copyright->setAlignment(AlignCenter);
  m_copyright->setText("(c) 2002 Heikki junnila");
      
  m_email = new QLabel(this);
  m_email->setGeometry(10, 50, 180, 20);
  m_email->setAlignment(AlignCenter);
  m_email->setText("heikki.junnila@iki.fi");

  m_ok = new QPushButton(this);
  m_ok->setGeometry(50, 160, 100, 30);
  m_ok->setMinimumSize(100, 30);
  m_ok->setDefault(false);
  m_ok->setText("&OK");
  connect(m_ok, SIGNAL(clicked()), this, SLOT(slotOKClicked()));
}

void AboutBox::slotOKClicked()
{
  accept();
}
