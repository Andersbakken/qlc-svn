/*
  Q Light Controller
  dmxaddresstool.h
  
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

#ifndef DMXADDRESSTOOL_H
#define DMXADDRESSTOOL_H

#include <qlabel.h>
#include <qlineedit.h>
#include <qdialog.h>
#include <qslider.h>
#include <qpushbutton.h>

class DMXAddressTool : public QDialog
{
  Q_OBJECT

 public:
  DMXAddressTool(QWidget* parent = NULL, const char* name = NULL);
  ~DMXAddressTool();

 public slots:
  void slotSetDipValue();
  void slotSetDecimalValue();

 protected:
  QLabel* m_qlabel1;
  QLabel* m_qlabel2;
  QLabel* m_qlabel3;
  QLabel* m_qlabel4;
  QLabel* m_qlabel5;
  QLabel* m_qlabel6;
  QLabel* m_qlabel7;
  QLabel* m_qlabel8;
  QLabel* m_qlabel9;
  QLabel* m_qlabel10;
  QLabel* m_qlabel11;
  QLabel* m_qlabel12;
  QLabel* m_qlabel13;

  QLineEdit* m_address;

  QSlider* m_256;
  QSlider* m_128;
  QSlider* m_64;
  QSlider* m_32;
  QSlider* m_16;
  QSlider* m_8;
  QSlider* m_4;
  QSlider* m_2;
  QSlider* m_1;

  QPushButton* m_toDecimal;
  QPushButton* m_toDip;

 private:
  void initDialog();
};

#endif









































