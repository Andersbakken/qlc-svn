/*
  Q Light Controller
  functioncollectioneditor.h
  
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

#ifndef FUNCTIONCOLLECTIONEDITOR_H
#define FUNCTIONCOLLECTIONEDITOR_H

#include <qdialog.h>

class QLineEdit;
class QLabel;
class QListView;
class QListViewItem;
class QPushButton;

class FunctionCollection;

class FunctionCollectionEditor : public QDialog
{
  Q_OBJECT

 public:
  FunctionCollectionEditor(FunctionCollection* functionCollection, QWidget* parent = NULL, const char* name = 0);
  ~FunctionCollectionEditor();

  QListViewItem* findItem(const unsigned long functionId);

  void show();

 private slots:
  void slotAddClicked();
  void slotRemoveClicked();
  void accept();

 private:
  void init();
  void updateFunctionList();

 protected:
  QListView* m_functionList;
  QLabel* m_nameLabel;
  QLineEdit* m_nameEdit;
  QPushButton* m_addButton;
  QPushButton* m_removeButton;
  QPushButton* m_editButton;
  QPushButton* m_ok;
  QPushButton* m_cancel;

  FunctionCollection* m_functionCollection;
};

#endif

