/*
  Q Light Controller
  joysticksettings.cpp

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

#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>

#include "joysticksettings.h"
#include "joystick.h"
#include "../common/ledbar.h"

JoystickSettings::JoystickSettings(QWidget* parent, QApplication* qapp, Joystick* j, const char* name)
  : QDialog(parent, name, false)
{
  ASSERT(j != NULL);

  m_joystick = j;

  m_qapp = qapp;

  initView();

  connect(j, SIGNAL(buttonEvent(int, int)), this, SLOT(slotButtonEvent(int, int)));
  connect(j, SIGNAL(axisEvent(int, int)), this, SLOT(slotAxisEvent(int, int)));

  connect(m_okButton, SIGNAL(clicked()), this, SLOT(slotOKClicked()));
}

JoystickSettings::~JoystickSettings()
{
}

void JoystickSettings::initView()
{
  setFixedSize(330, 350);
  setCaption("Joystick Settings");
  
  m_xyArea = new QFrame(this);
  m_xyArea->setGeometry(10, 10, 80, 80);
  m_xyArea->setPaletteBackgroundColor(QColor(255, 255, 255));
  m_xyArea->setFrameShape(QFrame::StyledPanel);
  m_xyArea->setFrameShadow(QFrame::Sunken);
  
  m_infoLabel = new QLabel(this);
  m_infoLabel->setGeometry(100, 10, 210, 80);
  m_infoLabel->setText("Move all of the joystick's axes to their limits and press each button to see that everything is working properly.");
  m_infoLabel->setTextFormat(QLabel::AutoText);
  m_infoLabel->setAlignment(QLabel::WordBreak | QLabel::AlignTop | QLabel::AlignTop | QLabel::AlignLeft);

  m_axesFrame = new QGroupBox(this);
  m_axesFrame->setGeometry(10, 160, 310, 155);
  m_axesFrame->setTitle("Axes");

  m_okButton = new QPushButton(this);
  m_okButton->setGeometry(220, 320, 90, 25);
  m_okButton->setText("OK");
  
  m_buttonsFrame = new QGroupBox(this);
  m_buttonsFrame->setGeometry(10, 100, 310, 50);
  m_buttonsFrame->setTitle("Buttons");

  QFont font;
  font.setStyleHint(QFont::System);
  font.setPointSize(8);

  for (unsigned int i = 0; i < m_joystick->buttonsList().count(); i++)
    {
      QString num;
      num.setNum(i + 1);
      QPushButton* b = new QPushButton(m_buttonsFrame);
      b->setGeometry(10 + (i * 20), 20, 20, 20);
      b->setText(num);
      b->setToggleButton(true);

      m_buttonsList.append(b);
    }

  for (unsigned int i = 0; i < m_joystick->axesList().count(); i++)
    {
      QString num;
      num.sprintf("%.3d", i + 1);
      LedBar* a = new LedBar(m_axesFrame, m_qapp);
      a->setGeometry(10 + (i * 25), 20, 20, 100);
      a->setText(num);
      a->setRange(0, 65535);
      a->setFont(font);
      a->setToolTip( m_joystick->axesList().at(i)->name );

      m_axesList.append(a);
    }

  for (unsigned int i = 0; i < m_joystick->axesList().count(); i++)
    {
      QPushButton* b = new QPushButton(m_axesFrame);
      b->setGeometry(10 + (i * 25), 125, 20, 20);
      b->setFont(font);
      b->setText("><");
      b->setToggleButton(true);
      QToolTip::add(b, "Invert axis");

      b->setOn( (m_joystick->axesList().at(i)->invert) ? true : false );

      connect(b, SIGNAL(released()), this, SLOT(slotInvertClicked()));

      m_invertsList.append(b);
    }
}

void JoystickSettings::slotInvertClicked()
{
  for (unsigned int i = 0; i < m_joystick->axesList().count(); i++)
    {
      if (m_invertsList.at(i)->isOn() == true)
	{
	  m_joystick->axesList().at(i)->invert = true;
	}
      else
	{
	  m_joystick->axesList().at(i)->invert = false;
	}
    }
}

void JoystickSettings::slotOKClicked()
{
  close();
}

void JoystickSettings::slotButtonEvent(int button, int value)
{
  QString foo;

  switch (value)
    {
    default:
    case 0:
      {
	m_qapp->lock();
	m_buttonsList.at(button)->setOn(false);
	m_qapp->unlock();
      }
      break;

    case 1:
      {
	m_qapp->lock();
	m_buttonsList.at(button)->setOn(true);
	m_qapp->unlock();
      }
      break;
    }
}

void JoystickSettings::slotAxisEvent(int axis, int value)
{
  value += 32767;

  if (m_joystick->axesList().at(axis)->invert == true)
    {
      m_axesList.at(axis)->slotSetValue(65535 - value);
    }
  else
    {
      m_axesList.at(axis)->slotSetValue(value);
    }
}
