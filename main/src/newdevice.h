/*
  Q Light Controller
  newdevice.h
  
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

#ifndef NEWDEVICE_H
#define NEWDEVICE_H

#include <qwidget.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qevent.h>
#include <qdialog.h>
#include <qcheckbox.h>

#include "classes.h"

class NewDevice : public QDialog
{
  Q_OBJECT

 public: 
  NewDevice(QWidget *parent = 0, const char *name = 0);
  ~NewDevice();

  QString manufacturer() const { return m_manufacturerValue; }

  QString model() const { return m_modelValue; }

  int address() const { return m_addressValue; }

  QString name() const { return m_nameValue; }

 protected:
  void closeEvent(QCloseEvent*);
  void init();
  void show();

  void showModels(QString mfg);
  void showManufacturers();

  QComboBox* m_manufacturer;
  QComboBox* m_model;
  QLineEdit* m_name;

  QLabel* m_qlabel1;
  QLabel* m_qlabel2;
  QLabel* m_qlabel3;
  QLabel* m_qlabel4;

  QSpinBox* m_channels;
  QSpinBox* m_address;

  QCheckBox* m_autoAddress;

  QPushButton* m_deviceInfo;
  QPushButton* m_ok;
  QPushButton* m_cancel;

 private:
  QString m_nameValue;
  QString m_manufacturerValue;
  QString m_modelValue;
  int m_addressValue;

 private slots:
  void slotOKClicked();
  void slotCancelClicked();
  void slotManufacturerActivated(int item);
  void slotModelActivated(int item);
  void slotNameChanged(const QString &text);
  void slotAutoAddressClicked();
};

#endif
