/*
  Q Light Controller
  dmxaddresstool.cpp
  
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

#include "dmxaddresstool.h"

DMXAddressTool::DMXAddressTool(QWidget* parent, const char* name) : QDialog(parent, name, true)
{
  initDialog();
}

DMXAddressTool::~DMXAddressTool()
{

}

void DMXAddressTool::initDialog()
{
  setCaption("DMX Address Tool");
  resize(220, 150);

  m_address = new QLineEdit(this);
  m_address->setGeometry(110, 10, 60, 20);
  m_address->setText("0");

  m_toDecimal = new QPushButton(this);
  m_toDecimal->setGeometry(170, 10, 20, 20);
  m_toDecimal->setText("<");
  connect(m_toDecimal, SIGNAL(clicked()), this, SLOT(slotSetDecimalValue()));

  m_toDip = new QPushButton(this);
  m_toDip->setGeometry(190, 10, 20, 20);
  m_toDip->setText(">");
  connect(m_toDip, SIGNAL(clicked()), this, SLOT(slotSetDipValue()));

  m_qlabel1 = new QLabel(this);
  m_qlabel1->setGeometry(10, 10, 100, 20);
  m_qlabel1->setText("DMX Address");

  m_qlabel2 = new QLabel(this);
  m_qlabel2->setGeometry(0, 40, 220, 20);
  m_qlabel2->setAlignment(AlignHCenter);
  m_qlabel2->setText("DIP Setting");

  m_256 = new QSlider(this);
  m_256->setGeometry(10, 60, 20, 60);
  m_256->setOrientation(QSlider::Vertical);
  m_256->setTracking(true);
  m_256->setRange(0, 1);

  m_qlabel3 = new QLabel(this);
  m_qlabel3->setGeometry(10, 120, 20, 30);
  m_qlabel3->setText("256");
  m_qlabel3->setAlignment(AlignHCenter | AlignBottom);

  m_128 = new QSlider(this);
  m_128->setGeometry(30, 60, 20, 60);
  m_128->setOrientation(QSlider::Vertical);
  m_128->setTracking(true);
  m_128->setRange(0, 1);

  m_qlabel4 = new QLabel(this);
  m_qlabel4->setGeometry(30, 120, 20, 30);
  m_qlabel4->setText("128");
  m_qlabel4->setAlignment(AlignHCenter | AlignTop);

  m_64 = new QSlider(this);
  m_64->setGeometry(50, 60, 20, 60);
  m_64->setOrientation(QSlider::Vertical);
  m_64->setTracking(true);
  m_64->setRange(0, 1);

  m_qlabel5 = new QLabel(this);
  m_qlabel5->setGeometry(50, 120, 20, 30);
  m_qlabel5->setText("64");
  m_qlabel5->setAlignment(AlignHCenter | AlignBottom);

  m_32 = new QSlider(this);
  m_32->setGeometry(70, 60, 20, 60);
  m_32->setOrientation(QSlider::Vertical);
  m_32->setTracking(true);
  m_32->setRange(0, 1);

  m_qlabel6 = new QLabel(this);
  m_qlabel6->setGeometry(70, 120, 20, 30);
  m_qlabel6->setText("32");
  m_qlabel6->setAlignment(AlignHCenter | AlignTop);

  m_16 = new QSlider(this);
  m_16->setGeometry(90, 60, 20, 60);
  m_16->setOrientation(QSlider::Vertical);
  m_16->setTracking(true);
  m_16->setRange(0, 1);

  m_qlabel7 = new QLabel(this);
  m_qlabel7->setGeometry(90, 120, 20, 30);
  m_qlabel7->setText("16");
  m_qlabel7->setAlignment(AlignHCenter | AlignBottom);

  m_8 = new QSlider(this);
  m_8->setGeometry(110, 60, 20, 60);
  m_8->setOrientation(QSlider::Vertical);
  m_8->setTracking(true);
  m_8->setRange(0, 1);

  m_qlabel8 = new QLabel(this);
  m_qlabel8->setGeometry(110, 120, 20, 30);
  m_qlabel8->setText("8");
  m_qlabel8->setAlignment(AlignHCenter | AlignTop);

  m_4 = new QSlider(this);
  m_4->setGeometry(130, 60, 20, 60);
  m_4->setOrientation(QSlider::Vertical);
  m_4->setTracking(true);
  m_4->setRange(0, 1);

  m_qlabel9 = new QLabel(this);
  m_qlabel9->setGeometry(130, 120, 20, 30);
  m_qlabel9->setText("4");
  m_qlabel9->setAlignment(AlignHCenter | AlignBottom);

  m_2 = new QSlider(this);
  m_2->setGeometry(150, 60, 20, 60);
  m_2->setOrientation(QSlider::Vertical);
  m_2->setTracking(true);
  m_2->setRange(0, 1);

  m_qlabel10 = new QLabel(this);
  m_qlabel10->setGeometry(150, 120, 20, 30);
  m_qlabel10->setText("2");
  m_qlabel10->setAlignment(AlignHCenter | AlignTop);

  m_1 = new QSlider(this);
  m_1->setGeometry(170, 60, 20, 60);
  m_1->setOrientation(QSlider::Vertical);
  m_1->setTracking(true);
  m_1->setRange(0, 1);

  m_qlabel11 = new QLabel(this);
  m_qlabel11->setGeometry(170, 120, 20, 30);
  m_qlabel11->setText("1");
  m_qlabel11->setAlignment(AlignHCenter | AlignBottom);

  m_qlabel12 = new QLabel(this);
  m_qlabel12->setGeometry(190, 60, 20, 20);
  m_qlabel12->setText("Off");

  m_qlabel13 = new QLabel(this);
  m_qlabel13->setGeometry(190, 100, 20, 20);
  m_qlabel13->setText("On");
}

void DMXAddressTool::slotSetDecimalValue()
{
  int number = 0;
  QString str;

  if (m_256->value() == 1) number += 256;
  if (m_128->value() == 1) number += 128;
  if (m_64->value() == 1) number += 64;
  if (m_32->value() == 1) number += 32;
  if (m_16->value() == 1) number += 16;
  if (m_8->value() == 1) number += 8;
  if (m_4->value() == 1) number += 4;
  if (m_2->value() == 1) number += 2;
  if (m_1->value() == 1) number += 1;

  str.setNum(number);
  m_address->setText(str);
}

void DMXAddressTool::slotSetDipValue()
{
  int number = 0;
  number = m_address->text().toInt();

  if (number < 0)
    {
      m_address->setText("0");
      number = 0;
    }
  else if (number > 511)
    {
      m_address->setText("511");
      number = 511;
    }
  
  number -= 256;
  if (number >= 0)
    {
      m_256->setValue(1);
    }
  else
    {
      number += 256;
      m_256->setValue(0);
    }
  
  number -= 128;
  if (number >= 0)
    {
      m_128->setValue(1);
    }
  else 
    {
      number += 128;
      m_128->setValue(0);
    }
  
  number -= 64;
  if (number >= 0)
    {
      m_64->setValue(1);
    }
  else
    {
      number += 64;
      m_64->setValue(0);
    }
  
  number -= 32;
  if (number >= 0)
    {
      m_32->setValue(1);
    }
  else
    {
      number += 32;
      m_32->setValue(0);
    }
  
  number -= 16;
  if (number >= 0)
    {
      m_16->setValue(1);
    }
  else
    {
      number += 16;
      m_16->setValue(0);
    }
  
  number -= 8;
  if (number >= 0)
    {
      m_8->setValue(1);
    }
  else
    {
      number += 8;
      m_8->setValue(0);
    }
  
  number -= 4;
  if (number >= 0)
    {
      m_4->setValue(1);
    }
  else
    {
      number += 4;
      m_4->setValue(0);
    }
  
  number -= 2;
  if (number >= 0)
    {
      m_2->setValue(1);
    }
  else
    {
      number += 2;
      m_2->setValue(0);
    }
  
  number -= 1;
  if (number >= 0)
    {
      m_1->setValue(1);
    }
  else
    {
      number += 1;
      m_1->setValue(0);
    }
}
