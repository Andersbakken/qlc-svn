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
#include "app.h"
#include "doc.h"
#include "deviceclass.h"
#include "dmxdevice.h"
#include "function.h"
#include "functiontree.h"

extern App* _app;

FunctionCollectionEditor::FunctionCollectionEditor(FunctionCollection* functionCollection, QWidget* parent, const char* name)
  : QDialog(parent, name, true)
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
  m_nameLabel->setGeometry(10, 10, 100, 30);
  m_nameLabel->setText("Function:");

  m_nameEdit = new QLineEdit(this);
  m_nameEdit->setGeometry(100, 10, 260, 30);
  m_nameEdit->setText(m_functionCollection->name());
  m_nameEdit->setFocus();
  m_nameEdit->setSelection(0, m_functionCollection->name().length());

  m_functionList = new QListView(this);
  m_functionList->setGeometry(10, 50, 350, 240);
  m_functionList->addColumn("Device");
  m_functionList->addColumn("Function");
  m_functionList->setColumnWidth(0, (int) (350 * 0.49));
  m_functionList->setColumnWidth(1, (int) (350 * 0.49));
  m_functionList->setAllColumnsShowFocus(true);

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
  FunctionTree* ft;
  ft = new FunctionTree(this);
  if (ft->exec() == QDialog::Accepted && ft->deviceString() != QString::null && ft->functionString() != QString::null)
    {
      if (findItem(ft->deviceString(), ft->functionString()) == NULL)
	{
	  new QListViewItem(m_functionList, ft->deviceString(), ft->functionString());

	  DMXDevice* device = _app->doc()->searchDevice(ft->deviceString(), DeviceClass::ANY);

	  Function* function = device->searchFunction(ft->functionString());
	  if (function == NULL)
	    {
	      function = device->deviceClass()->searchFunction(ft->functionString());
	    }

	  ASSERT(function != NULL);
	  ASSERT(m_functionCollection != NULL);

	  m_functionCollection->addItem(device, function);
	}
      else
	{
	  MSG_WARN("The selected device/function is already present in the current collection.\nOperation aborted.");
	}
    }
}

void FunctionCollectionEditor::slotRemoveClicked()
{
  if (m_functionList->selectedItem() != NULL)
    {
      QString functionString;
      QString deviceString;

      deviceString = m_functionList->selectedItem()->text(0);
      functionString = m_functionList->selectedItem()->text(1);

      m_functionCollection->removeItem(deviceString, functionString);
      m_functionList->takeItem(m_functionList->selectedItem());
    }
}

void FunctionCollectionEditor::updateFunctionList()
{
  ASSERT(m_functionCollection != NULL);
  QList <CollectionItem> il = m_functionCollection->items();

  m_functionList->clear();

  for (CollectionItem* item = il.first(); item != NULL; item = il.next())
    {
      new QListViewItem(m_functionList, item->callerDevice->name(), item->feederFunction->name());
    }
}

QListViewItem* FunctionCollectionEditor::findItem(QString deviceString, QString functionString)
{
  QListViewItem* item = NULL;

  QListViewItemIterator it(m_functionList);
  for (it = it; it.current(); ++it)
    {
      if (it.current()->text(0) == deviceString && it.current()->text(1) == functionString)
	{
	  item = it.current();
	  break;
	}
    }

  return item;
}
