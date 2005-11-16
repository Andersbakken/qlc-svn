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

#include "types.h"
#include "uic_newdevice.h"

class QString;

class NewDevice : public UI_NewDevice
{
  Q_OBJECT

 public: 
  NewDevice(QWidget *parent = 0, const char *name = 0);
  ~NewDevice();

  QString manufacturer() const { return m_manufacturerValue; }
  QString model() const { return m_modelValue; }
  t_channel address() const { return m_addressValue; }
  t_channel universe() const { return m_universeValue; }
  QString name() const { return m_nameValue; }

 protected:
  void initView();
  void show();

  void fillTree();

 private:
  QString m_nameValue;
  QString m_typeValue;
  QString m_manufacturerValue;
  QString m_modelValue;
  t_channel m_addressValue;
  t_channel m_universeValue;
  bool m_selectionOK;

 private slots:
  void slotOKClicked();
  void slotCancelClicked();
  void slotSelectionChanged(QListViewItem* item);
  void slotNameChanged(const QString &text);
  void slotTreeDoubleClicked(QListViewItem* item);
  void slotTreeOpenCheckBoxClicked();
  void slotDIPClicked();
};

#endif
