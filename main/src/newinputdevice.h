/*
  Q Light Controller
  newinputdevice.h

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

#ifndef NEWINPUTDEVICE_H
#define NEWINPUTDEVICE_H

#include <qdialog.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>

class NewInputDevice : public QDialog
{
  Q_OBJECT

 public:
  NewInputDevice(QWidget* parent, const char* name = NULL);
  ~NewInputDevice();

 private:
  void initView();
  void fetchInputDevices();

 private slots:
  void slotSelectionChanged(QListViewItem* item);
  void slotCustomTextChanged(const QString &text);

 protected:
  QListView* m_list;
  QLabel* m_customNameLabel;
  QLineEdit* m_customNameEdit;
  QPushButton* m_ok;
  QPushButton* m_cancel;

  QString m_customNameValue;
  QString m_nameValue;
  QString m_typeValue;
};

#endif

