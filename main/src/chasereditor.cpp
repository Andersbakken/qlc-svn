/*
  Q Light Controller
  chasereditor.cpp
  
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

#include "chasereditor.h"
#include "deviceclass.h"
#include "function.h"
#include "dmxdevice.h"
#include "app.h"
#include "doc.h"
#include "functiontree.h"
#include "chaser.h"
#include <stdlib.h>
#include <qlistview.h>

extern App* _app;

ChaserEditor::ChaserEditor(Chaser* function, QWidget* parent, const char* name)
  : QDialog(parent, name, true)
{
  m_function = function;
  ASSERT(m_function != NULL);

  m_prevItem = NULL;

  init();
  addItems();
}

ChaserEditor::~ChaserEditor()
{

}

void ChaserEditor::addItems()
{
  ChaserStep* step = NULL;
  QListViewItem* prev = NULL;
  QString deviceName;

  int steps = m_function->steps();

  for (int i = 0; i < steps; i++)
    {
      step = m_function->at(i);
      if (prev == NULL)
	{
	  if (step->callerDevice != NULL)
	    {
	      deviceName = step->callerDevice->name();
	    }
	  else
	    {
	      deviceName = QString("Global");
	    }

	  prev = new QListViewItem(m_functionList, "", deviceName, step->feederFunction->name());
	}
      else
	{
	  if (step->callerDevice != NULL)
	    {
	      deviceName = step->callerDevice->name();
	    }
	  else
	    {
	      deviceName = QString("Global");
	    }

	  prev = new QListViewItem(m_functionList, prev, "", deviceName, step->feederFunction->name());
	}

      m_prevItem = prev;
    }

  updateOrderNumbers();
}

void ChaserEditor::init()
{
  setCaption("Chaser Editor");
  resize(520, 300);
  setFixedSize(520, 300);

  m_nameLabel = new QLabel(this);
  m_nameLabel->setGeometry(10, 10, 100, 30);
  m_nameLabel->setText("Name:");

  m_nameEdit = new QLineEdit(this);
  m_nameEdit->setGeometry(100, 10, 260, 30);
  m_nameEdit->setText(m_function->name());
  m_nameEdit->setFocus();
  m_nameEdit->setSelection(0, m_function->name().length());

  m_functionList = new QListView(this);
  m_functionList->setGeometry(10, 50, 350, 230);
  m_functionList->setSorting(-1);
  m_functionList->addColumn("#");
  m_functionList->addColumn("Device");
  m_functionList->addColumn("Function");
  m_functionList->setColumnWidth(0, (int) (350 * 0.09));
  m_functionList->setColumnWidth(1, (int) (350 * 0.45));
  m_functionList->setColumnWidth(2, (int) (350 * 0.45));
  m_functionList->setAllColumnsShowFocus(true);

  m_raiseButton = new QToolButton(UpArrow, this);
  m_raiseButton->setGeometry(360, 50, 20, 115);
  connect(m_raiseButton, SIGNAL(clicked()), this, SLOT(slotRaiseClicked()));

  m_lowerButton = new QToolButton(DownArrow, this);
  m_lowerButton->setGeometry(360, 165, 20, 115);
  connect(m_lowerButton, SIGNAL(clicked()), this, SLOT(slotLowerClicked()));
  
  m_addButton = new QPushButton(this);
  m_addButton->setText("&Add Step");
  m_addButton->setGeometry(390, 50, 120, 30);
  connect(m_addButton, SIGNAL(clicked()), this, SLOT(slotAddClicked()));

  m_removeButton = new QPushButton(this);
  m_removeButton->setText("&Remove step");
  m_removeButton->setGeometry(390, 90, 120, 30);
  connect(m_removeButton, SIGNAL(clicked()), this, SLOT(slotRemoveClicked()));

  m_editButton = new QPushButton(this);
  m_editButton->setText("&Edit step");
  m_editButton->setGeometry(390, 130, 120, 30);
  connect(m_editButton, SIGNAL(clicked()), this, SLOT(slotEditClicked()));

  m_ok = new QPushButton(this);
  m_ok->setText("&OK");
  m_ok->setGeometry(390, 220, 120, 30);
  connect(m_ok, SIGNAL(clicked()), this, SLOT(slotOKClicked()));

  m_cancel = new QPushButton(this);
  m_cancel->setText("&Cancel");
  m_cancel->setGeometry(390, 260, 120, 30);
  connect(m_cancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
}

void ChaserEditor::slotOKClicked()
{
  // Remove all existing steps
  int steps = m_function->steps();
  for (int i = 0; i < steps; i++)
    {
      m_function->removeStep(0);
    }

  // Insert new steps
  QListViewItemIterator it(m_functionList);
  QString functionString;
  QString deviceString;
  DMXDevice* device = NULL;
  Function* function = NULL;

  // Iterate through all items of the listview
  for (; it.current() != NULL; ++it)
    {
      deviceString = it.current()->text(1);
      functionString = it.current()->text(2);

      if (deviceString == QString("Global"))
	{
	  device = NULL;
	}
      else
	{
	  device = _app->doc()->searchDevice(deviceString);
	  ASSERT(device != NULL);
	}

      if (device == NULL)
	{
	  function = _app->doc()->searchFunction(functionString);
	}
      else
	{
	  function = device->searchFunction(functionString);
	  if (function == NULL)
	    {
	      function = device->deviceClass()->searchFunction(functionString);
	    }
	}

      ASSERT(function != NULL);

      m_function->addStep(device, function);
    }

  m_function->setName(m_nameEdit->text());

  accept();
}

void ChaserEditor::slotCancelClicked()
{
  reject();
}

void ChaserEditor::slotEditClicked()
{
}

void ChaserEditor::slotRemoveClicked()
{
  QListViewItem* item = m_functionList->currentItem();

  if (item != NULL)
    {
      m_functionList->takeItem(item);
      delete item;
    }
  updateOrderNumbers();
}

void ChaserEditor::slotAddClicked()
{
  FunctionTree* ft = new FunctionTree(this);
  if (ft->exec() == QDialog::Accepted && ft->deviceString() != QString::null && ft->functionString() != QString::null)
    {
      if (m_prevItem == NULL)
	{
	  m_prevItem = new QListViewItem(m_functionList, "", ft->deviceString(), ft->functionString());
	}
      else
	{
	  m_prevItem = new QListViewItem(m_functionList, m_prevItem, "", ft->deviceString(), ft->functionString());
	}
    }

  updateOrderNumbers();
}

void ChaserEditor::updateOrderNumbers()
{
  int i = 0;
  QString num;

  // Create an iterator and give the listview as argument
  QListViewItemIterator it(m_functionList);

  // Iterate through all items of the listview
  for (; it.current() != NULL; ++it)
    {
      num.setNum(i++);
      it.current()->setText(0, num);
    }
}

void ChaserEditor::slotRaiseClicked()
{
  QListViewItem* currentItem = m_functionList->currentItem();

  if (currentItem != NULL)
    {
      QString deviceString = currentItem->text(1);
      QString functionString = currentItem->text(2);

      QListViewItem* above = NULL;
      if (currentItem->itemAbove() != NULL && currentItem->itemAbove()->itemAbove() != NULL)
	{
	  above = currentItem->itemAbove()->itemAbove();
	}

      m_functionList->takeItem(currentItem);
      delete currentItem;

      QListViewItem* newItem;
      if (above != NULL)
	{
	  newItem = new QListViewItem(m_functionList, above, "", deviceString, functionString);
	}
      else
	{
	  newItem = new QListViewItem(m_functionList, "", deviceString, functionString);
	}

      m_functionList->setSelected(newItem, true);
    }

  updateOrderNumbers();
}

void ChaserEditor::slotLowerClicked()
{
  QListViewItem* currentItem = m_functionList->currentItem();

  if (currentItem != NULL)
    {
      QString deviceString = currentItem->text(1);
      QString functionString = currentItem->text(2);

      QListViewItem* below = NULL;
      if (currentItem->itemBelow() != NULL)
	{
	  below = currentItem->itemBelow();
	}

      if (below != NULL)
	{
	  m_functionList->takeItem(currentItem);
	  delete currentItem;
	  QListViewItem* newItem;
	  newItem = new QListViewItem(m_functionList, below, "", deviceString, functionString);
	  m_functionList->setSelected(newItem, true);
	}
    }
  updateOrderNumbers();
}
