/*
  Q Light Controller
  functioncollectioneditor.cpp
  
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

#include "functioncollectioneditor.h"
#include "functioncollection.h"
#include "function.h"
#include "functionstep.h"
#include "functiontree.h"
#include "app.h"
#include "doc.h"
#include "deviceclass.h"
#include "device.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

extern App* _app;

FunctionCollectionEditor::FunctionCollectionEditor(FunctionCollection* functionCollection, QWidget* parent)
  : QDialog(parent, "", true)
{
  ASSERT(functionCollection != NULL);
  m_functionCollection = functionCollection;

  init();
}

FunctionCollectionEditor::~FunctionCollectionEditor()
{
  
}

void FunctionCollectionEditor::init()
{
  resize(500, 300);
  setFixedSize(500, 300);

  setCaption("Function Collection");
  
  m_nameLabel = new QLabel(this);
  m_nameLabel->setGeometry(10, 10, 130, 30);
  m_nameLabel->setText("Collection Name");

  m_nameEdit = new QLineEdit(this);
  m_nameEdit->setGeometry(130, 10, 230, 30);
  m_nameEdit->setText(m_functionCollection->name());
  m_nameEdit->setFocus();
  m_nameEdit->setSelection(0, m_functionCollection->name().length());

  m_functionList = new QListView(this);
  m_functionList->setGeometry(10, 50, 350, 240);
  m_functionList->addColumn("Device");
  m_functionList->addColumn("Function");
  m_functionList->addColumn("Function ID");
  m_functionList->setAllColumnsShowFocus(true);
  m_functionList->setResizeMode(QListView::LastColumn);

  m_addButton = new QPushButton(this);
  m_addButton->setText("Add");
  m_addButton->setGeometry(380, 50, 100, 30);
  connect(m_addButton, SIGNAL(clicked()), this, SLOT(slotAddClicked()));
  
  m_removeButton = new QPushButton(this);
  m_removeButton->setText("Remove");
  m_removeButton->setGeometry(380, 90, 100, 30);
  connect(m_removeButton, SIGNAL(clicked()), this, SLOT(slotRemoveClicked()));

  m_ok = new QPushButton(this);
  m_ok->setText("OK");
  m_ok->setGeometry(380, 210, 100, 30);
  connect(m_ok, SIGNAL(clicked()), this, SLOT(accept()));
  
  m_cancel = new QPushButton(this);
  m_cancel->setText("Cancel");
  m_cancel->setGeometry(380, 250, 100, 30);
  connect(m_cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void FunctionCollectionEditor::show()
{
  updateFunctionList();

  QDialog::show();
}

void FunctionCollectionEditor::accept()
{
  m_functionCollection->setName(m_nameEdit->text());

  QDialog::accept();
}

void FunctionCollectionEditor::slotAddClicked()
{
  FunctionTree* ft = new FunctionTree(this);

  if (ft->exec() == QDialog::Accepted && ft->functionID() != 0)
    {
      if (findItem(ft->functionID()) == NULL)
	{
	  Function* function = NULL;
	  function = _app->doc()->searchFunction(ft->functionID());
	  ASSERT(function != NULL);
	  
	  QString id;
	  id.setNum(function->id());
	  new QListViewItem(m_functionList, QString("Global"), 
			    function->name(), id);

	  m_functionCollection->addItem(function);
	}
      else
	{
	  QString msg("The selected function is already in collection.");
	  QMessageBox::warning(this, KApplicationNameShort, msg);
	}
    }
}

void FunctionCollectionEditor::slotRemoveClicked()
{
  if (m_functionList->selectedItem() != NULL)
    {
      t_function_id id = 0;

      id = m_functionList->selectedItem()->text(2).toULong();

      m_functionCollection->removeItem(id);
      m_functionList->takeItem(m_functionList->selectedItem());
    }
}

void FunctionCollectionEditor::updateFunctionList()
{
  ASSERT(m_functionCollection != NULL);
  QPtrList <FunctionStep> *il = m_functionCollection->steps();

  m_functionList->clear();

  for (FunctionStep* item = il->first(); item != NULL; item = il->next())
    {
      QString id;
      id.setNum(item->function()->id());
      new QListViewItem(m_functionList, item->function()->device()->name(), item->function()->name(), id);
    }
}

QListViewItem* FunctionCollectionEditor::findItem(const t_function_id functionId)
{
  QListViewItem* item = NULL;
  QListViewItemIterator it(m_functionList);

  QString id;
  id.setNum(functionId);

  for (it = it; it.current(); ++it)
    {
      if (it.current()->text(2) == id)
	{
	  item = it.current();
	  break;
	}
    }

  return item;
}
