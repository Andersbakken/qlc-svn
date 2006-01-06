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

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qfont.h>

#include "aboutbox.h"
#include "app.h"
#include "configkeys.h"

#include "common/settings.h"

extern App* _app;

AboutBox::AboutBox(QWidget* parent, const char* name)
  : QDialog (parent, name, true)
{
  initDialog();
}

AboutBox::~AboutBox()
{
  delete m_pm;
  delete m_logo;
  delete m_version;
  delete m_copyright;
  delete m_people;
  delete m_ok;
}

void AboutBox::initDialog()
{
  QColor white(255, 255, 255);
  int w = 350;
  int h = 123;

  setCaption(QString("About ") + KApplicationNameLong);

  m_pm = new QPixmap(QString(PIXMAPS) + QString("/qlc-big.xpm"));
  m_logo = new QLabel(this);

  if (m_pm->isNull() == false)
    {
      m_logo->setGeometry(50, 0, 254, h);
      m_logo->setPixmap(*m_pm);
    }
  else
    {
      m_logo->setGeometry(0, 0, w, h);
      m_logo->setBackgroundColor(QColor(255, 255, 255));
      m_logo->setAlignment(AlignCenter);
      m_logo->setText("Incorrect pixmap path!");
    }

  setFixedSize(w, h + 220);
  setBackgroundColor(white);

  QFont font;
  font.setPixelSize(12);

  m_version = new QLabel(this);
  m_version->setFont(font);
  m_version->setGeometry(0, h, w, 20);
  m_version->setAlignment(AlignCenter);
  m_version->setText(KApplicationVersion);
  m_version->setBackgroundColor(white);

  m_copyright = new QLabel(this);
  m_copyright->setFont(font);
  m_copyright->setGeometry(0, h + 30, w, 50);
  m_copyright->setAlignment(AlignCenter);
  m_copyright->setTextFormat(RichText);
  m_copyright->setText("QLC is licensed under the GNU GPL<BR>and contains &copy; material created by<BR><B>the people involved in QLC development:</B>");
  m_copyright->setBackgroundColor(white);

  m_people = new QListBox(this);
  m_people->setFont(font);
  m_people->setGeometry(5, h + 90, w - 10, 80);
  m_people->setFocusPolicy(QWidget::NoFocus);
  m_people->setFrameStyle(QFrame::Panel);
  m_people->setScrollBar(true);
  m_people->insertItem("Heikki Junnila <hjunnila@users.sf.net>");
  m_people->insertItem("Stefan Krumm <stefankrumm@users.sf.net>");
  m_people->insertItem("Christian Sühs <dance-or-die@users.sf.net>");
  m_people->setBackgroundColor(white);

  m_ok = new QPushButton(this);
  m_ok->setGeometry(133, h + 180, 100, 30);
  m_ok->setMinimumSize(100, 30);
  m_ok->setDefault(false);
  m_ok->setText("&OK");
  m_ok->setBackgroundColor(white);
  connect(m_ok, SIGNAL(clicked()), this, SLOT(slotOKClicked()));
}

void AboutBox::slotOKClicked()
{
  accept();
}
