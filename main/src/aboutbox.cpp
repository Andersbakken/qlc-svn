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
#include "app.h"
#include "settings.h"

extern App* _app;

AboutBox::AboutBox(QWidget* parent, const char* name) : QDialog (parent, name, true)
{
  initDialog();
}

AboutBox::~AboutBox()
{

}

void AboutBox::initDialog()
{
  QColor white(255, 255, 255);
  int w = 0;
  int h = 0;

  setCaption("About QLC");

  m_pm = new QPixmap(_app->settings()->getPixmapPath() + "/qlc-big.xpm");
  if (m_pm->isNull() == false)
    {
      w = m_pm->width();
      h = m_pm->height();
      m_logo = new QWidget(this);
      m_logo->setGeometry(0, 0, w, h);
      m_logo->setBackgroundPixmap(*m_pm);
    }
  else
    {
      w = 254;
      h = 123;
      m_logo = (QWidget*) new QLabel(this);
      m_logo->setGeometry(0, 0, w, h);
      m_logo->setBackgroundColor(QColor(255, 255, 255));
      ((QLabel*) m_logo)->setAlignment(AlignCenter);
      ((QLabel*) m_logo)->setText("Pixmap path is missing from your settings!");
    }

  setFixedSize(w, h + 220);
  setBackgroundColor(white);

  m_version = new QLabel(this);
  m_version->setGeometry(0, h, w, 20);
  m_version->setAlignment(AlignCenter);
  m_version->setText("Version 2.0.10");
  m_version->setBackgroundColor(white);
  
  m_copyright = new QLabel(this);
  m_copyright->setGeometry(0, h + 20, w, 20);
  m_copyright->setAlignment(AlignCenter);
  m_copyright->setText("(c) 2002 Heikki junnila");
  m_copyright->setBackgroundColor(white);

  m_peopleLabel = new QLabel(this);
  m_peopleLabel->setGeometry(5, h + 70, w - 10, 20);
  m_peopleLabel->setAlignment(AlignLeft);
  m_peopleLabel->setText("People involved in qlc development:");
  m_peopleLabel->setBackgroundColor(white);

  m_people = new QListBox(this);
  m_people->setGeometry(5, h + 90, w-10, 80);
  m_people->insertItem("Heikki Junnila (hjunnila@iki.fi)");
  m_people->insertItem("Stefan Krumm (krumm@geol.uni-erlangen.de)");
  m_people->insertItem("Dirk Jagdmann (doj@cubic.org)");

  m_ok = new QPushButton(this);
  m_ok->setGeometry(80, h + 180, 100, 30);
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
