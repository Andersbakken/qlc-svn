/*
  Q Light Controller
  joysticksettings.h

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

#ifndef JOYSTICKSETTINGS_H
#define JOYSTICKSETTINGS_H

#include <qdialog.h>
#include <qlist.h>
#include <qpushbutton.h>
#include "joystick.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QFrame;
class QGroupBox;
class QLabel;
class QApplication;
class LedBar;

class JoystickSettings : public QDialog
{
  Q_OBJECT

 public:
  JoystickSettings(QWidget* parent, QApplication* qapp, Joystick* j, const char* name = NULL);
  ~JoystickSettings();

  void initView();

 private:
  Joystick* m_joystick;
  QList <QPushButton> m_buttonsList;
  QList <LedBar> m_axesList;
  QList <QPushButton> m_invertsList;

 private slots:
  void slotButtonEvent(int button, int value);
  void slotAxisEvent(int axis, int value);
  void slotInvertClicked();
  void slotOKClicked();

 protected:
  QApplication* m_qapp;

  QFrame* m_xyArea;
  QLabel* m_infoLabel;

  QGroupBox* m_axesFrame;
  QGroupBox* m_buttonsFrame;

  QPushButton* m_okButton;
};

#endif

