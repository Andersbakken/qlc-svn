/*
  Q Light Controller
  chasereditor.h
  
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

#ifndef CHASEREDITOR_H
#define CHASEREDITOR_H

#include <qdialog.h>

class QListView;
class QListViewItem;
class QLineEdit;
class QLabel;
class QPushButton;
class QToolButton;

class Chaser;

class ChaserEditor : public QDialog
{
  Q_OBJECT

 public:
  ChaserEditor(Chaser* function, QWidget* parent = NULL, const char* name = NULL);
  ~ChaserEditor();

  void init();

 private:
  void updateOrderNumbers();
  void addItems();

 private:
  Chaser* m_function;
  QListViewItem* m_prevItem;

 protected:
  QLabel* m_nameLabel;
  QLineEdit* m_nameEdit;

  QListView* m_functionList;
  QPushButton* m_addButton;
  QPushButton* m_removeButton;
  QPushButton* m_editButton;

  QToolButton* m_raiseButton;
  QToolButton* m_lowerButton;

  QPushButton* m_ok;
  QPushButton* m_cancel;

 private slots:
  void slotCancelClicked();
  void slotOKClicked();
  void slotEditClicked();
  void slotRemoveClicked();
  void slotAddClicked();
  void slotRaiseClicked();
  void slotLowerClicked();

};

#endif
