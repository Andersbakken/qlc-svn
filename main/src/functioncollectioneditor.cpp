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

FunctionCollectionEditor::FunctionCollectionEditor(FunctionCollection* fc,
						   QWidget* parent)
  : UI_FunctionCollectionEditor(parent, "FunctionCollectionEditor", true)
{
  ASSERT(fc);
  m_fc = new FunctionCollection(fc);
  m_original = fc;

  init();
}

FunctionCollectionEditor::~FunctionCollectionEditor()
{
  delete m_fc;
  m_fc = NULL;
}

void FunctionCollectionEditor::init()
{
  m_nameEdit->setText(m_fc->name());
  updateFunctionList();
}

void FunctionCollectionEditor::slotAddFunctionClicked()
{
  FunctionTree* ft = new FunctionTree(this);
  ASSERT(ft);

  if (ft->exec() == QDialog::Accepted && ft->functionID() != 0)
    {
      if (isAlreadyMember(ft->functionID()))
	{
	  QString msg("The selected function is already in collection.");
	  QMessageBox::warning(this, KApplicationNameShort, msg);
	}
      else
	{
      	  Function* function = NULL;
	  function = _app->doc()->searchFunction(ft->functionID());
	  ASSERT(function);
	  
	  QString id;
	  id.setNum(function->id());
	  new QListViewItem(m_functionList, QString("Global"), 
			    function->name(), id);

	  m_fc->addItem(function);
	}
    }

  delete ft;
}


//
// Remove a function
//
void FunctionCollectionEditor::slotRemoveFunctionClicked()
{
  if (m_functionList->selectedItem() != NULL)
    {
      t_function_id id = 0;

      id = m_functionList->selectedItem()->text(2).toULong();

      m_fc->removeItem(id);
      m_functionList->takeItem(m_functionList->selectedItem());
    }
}


//
// OK clicked; accept and save changes
//
void FunctionCollectionEditor::slotOKClicked()
{
  m_fc->setName(m_nameEdit->text());

  m_original->copyFrom(m_fc);

  _app->doc()->setModified(true);

  QDialog::accept();
}


//
// Cancel pressed; discard changes
//
void FunctionCollectionEditor::slotCancelClicked()
{
  reject();
}


//
// Fill the function list
//
void FunctionCollectionEditor::updateFunctionList()
{
  ASSERT(m_fc);
  QPtrListIterator <FunctionStep> it(*m_fc->steps());

  m_functionList->clear();

  while (it.current())
    {
      QString id;
      id.setNum(it.current()->function()->id());
      new QListViewItem(m_functionList, 
			it.current()->function()->device()->name(), 
			it.current()->function()->name(), id);
      ++it;
    }
}


//
// Find an item from function list
//
bool FunctionCollectionEditor::isAlreadyMember(t_function_id id)
{
  QPtrListIterator <FunctionStep> it(*m_fc->steps());

  while (it.current())
    {
      if (it.current()->function()->id() == id)
	{
	  return true;
	}

      ++it;
    }

  return false;
}
