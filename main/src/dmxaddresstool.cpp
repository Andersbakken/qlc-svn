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


#include <qlabel.h>
#include <qlineedit.h>
#include <qdialog.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qfont.h>

#include "dmxaddresstool.h"
#include "app.h"
#include "settings.h"

extern App* _app;

DMXAddressTool::DMXAddressTool(QWidget* parent, const char* name) 
  : UI_DMXAddressTool(parent, name, true)
{
  m_address = 0;

  m_updateValue = true;

  QFont font;
  font.setStyleHint(QFont::System);
  font.setPointSize(11);

  m_256Label->setFont(font);
  m_256Label->setText("256");
  connect(m_256Slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));

  m_128Label->setFont(font);
  m_128Label->setText("128");
  connect(m_128Slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));

  m_64Label->setFont(font);
  m_64Label->setText("64");
  connect(m_64Slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));

  m_32Label->setFont(font);
  m_32Label->setText("32");
  connect(m_32Slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));

  m_16Label->setFont(font);
  m_16Label->setText("16");
  connect(m_16Slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));

  m_8Label->setFont(font);
  m_8Label->setText("8");
  connect(m_8Slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));

  m_4Label->setFont(font);
  m_4Label->setText("4");
  connect(m_4Slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));

  m_2Label->setFont(font);
  m_2Label->setText("2");
  connect(m_2Slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));

  m_1Label->setFont(font);
  m_1Label->setText("1");
  connect(m_1Slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChanged(int)));

  connect(m_decimalEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotDecimalChanged(const QString &)));
}

DMXAddressTool::~DMXAddressTool()
{

}

void DMXAddressTool::slotDecimalChanged(const QString &text)
{
  if (m_updateValue == false)
    {
      return;
    }

  m_updateValue = false;

  int number = 0;
  number = m_decimalEdit->text().toInt();
  m_address = number;

  if (number < 0)
    {
      m_decimalEdit->setText("0");
      number = 0;
    }
  else if (number > 511)
    {
      m_decimalEdit->setText("511");
      number = 511;
    }
  
  number -= 256;
  if (number >= 0)
    {
      m_256Slider->setValue(1);
    }
  else
    {
      number += 256;
      m_256Slider->setValue(0);
    }
  
  number -= 128;
  if (number >= 0)
    {
      m_128Slider->setValue(1);
    }
  else 
    {
      number += 128;
      m_128Slider->setValue(0);
    }
  
  number -= 64;
  if (number >= 0)
    {
      m_64Slider->setValue(1);
    }
  else
    {
      number += 64;
      m_64Slider->setValue(0);
    }
  
  number -= 32;
  if (number >= 0)
    {
      m_32Slider->setValue(1);
    }
  else
    {
      number += 32;
      m_32Slider->setValue(0);
    }
  
  number -= 16;
  if (number >= 0)
    {
      m_16Slider->setValue(1);
    }
  else
    {
      number += 16;
      m_16Slider->setValue(0);
    }
  
  number -= 8;
  if (number >= 0)
    {
      m_8Slider->setValue(1);
    }
  else
    {
      number += 8;
      m_8Slider->setValue(0);
    }
  
  number -= 4;
  if (number >= 0)
    {
      m_4Slider->setValue(1);
    }
  else
    {
      number += 4;
      m_4Slider->setValue(0);
    }
  
  number -= 2;
  if (number >= 0)
    {
      m_2Slider->setValue(1);
    }
  else
    {
      number += 2;
      m_2Slider->setValue(0);
    }
  
  number -= 1;
  if (number >= 0)
    {
      m_1Slider->setValue(1);
    }
  else
    {
      number += 1;
      m_1Slider->setValue(0);
    }

  m_updateValue = true;
}

void DMXAddressTool::slotSliderValueChanged(int value)
{
  if (m_updateValue == false)
    {
      return;
    }

  m_updateValue = false;

  int number = 0;
  QString str;

  if (m_256Slider->value() == 1) number += 256;
  if (m_128Slider->value() == 1) number += 128;
  if (m_64Slider->value() == 1) number += 64;
  if (m_32Slider->value() == 1) number += 32;
  if (m_16Slider->value() == 1) number += 16;
  if (m_8Slider->value() == 1) number += 8;
  if (m_4Slider->value() == 1) number += 4;
  if (m_2Slider->value() == 1) number += 2;
  if (m_1Slider->value() == 1) number += 1;

  m_address = number;

  str.setNum(number);
  m_decimalEdit->setText(str);

  m_updateValue = true;
}

void DMXAddressTool::setAddress(int address)
{
  m_address = address;

  QString str;
  str.setNum(address);
  m_decimalEdit->setText(str);
}
