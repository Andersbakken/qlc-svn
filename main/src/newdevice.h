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

#include <qdialog.h>

class QListView;
class QListViewItem;
class QCheckBox;
class QLineEdit;
class QLabel;
class QPushButton;
class QSpinBox;
class QString;

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
  void initView();
  void show();

  void fillTree();

  QListView* m_tree;

  QCheckBox* m_treeOpenCheckBox;

  QLineEdit* m_nameEdit;
  QLabel* m_nameLabel;

  QLabel* m_typeLabel;

  QSpinBox* m_channelSpin;
  QLabel* m_channelLabel;

  QSpinBox* m_addressSpin;
  QLabel* m_addressLabel;

  QPushButton* m_dipButton;
  QPushButton* m_ok;
  QPushButton* m_cancel;

 private:
  QString m_nameValue;
  QString m_typeValue;
  QString m_manufacturerValue;
  QString m_modelValue;
  int m_addressValue;
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
