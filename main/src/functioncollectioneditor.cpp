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

#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <assert.h>
#include <qtimer.h>

#include "common/deviceclass.h"
#include "functioncollectioneditor.h"
#include "functioncollection.h"
#include "function.h"
#include "functionmanager.h"
#include "app.h"
#include "doc.h"
#include "device.h"
#include "configkeys.h"

extern App* _app;

FunctionCollectionEditor::FunctionCollectionEditor(FunctionCollection* fc,
						   QWidget* parent)
  : UI_FunctionCollectionEditor(parent, "FunctionCollectionEditor", true)
{
  ASSERT(fc);
  m_original = fc;

  m_fc = new FunctionCollection();
  m_fc->copyFrom(fc);

  m_functionManager = NULL;

  init();
}

FunctionCollectionEditor::~FunctionCollectionEditor()
{
  delete m_fc;
  m_fc = NULL;
}

void FunctionCollectionEditor::init()
{
  m_addFunction->setPixmap(QPixmap(QString(PIXMAPS) + QString("/edit_add.png")));
  m_removeFunction->setPixmap(QPixmap(QString(PIXMAPS) + QString("/edit_remove.png")));

  m_nameEdit->setText(m_fc->name());
  updateFunctionList();
}

/**
 * The user wants to add functions
 */
void FunctionCollectionEditor::slotAddFunctionClicked()
{
	if (m_functionManager == NULL)
	{
		// Create the function manager in selection mode so it
		// looks like a normal modal dialog
		m_functionManager = new FunctionManager(this,
					FunctionManager::SelectionMode);

		// Prevent the user from selecting this function
		m_functionManager->setInactiveID(m_original->id());
		m_functionManager->init();

		// Catch the close event
		connect(m_functionManager, SIGNAL(closed()),
			this, SLOT(slotFunctionManagerClosed()));
	}
	
	// Show the dialog
	m_functionManager->show();
}

/**
 * The selection dialog was closed, take the selection
 */
void FunctionCollectionEditor::slotFunctionManagerClosed()
{
	FunctionIDList list;
	FunctionIDList::iterator it;

	assert(m_functionManager);

	if (m_functionManager->result() == QDialog::Accepted)
	{
		m_functionManager->selection(list);

		for (it = list.begin(); it != list.end(); ++it)
		{
			if (isAlreadyMember(*it) == false)
			{
				m_fc->addItem(*it);
			}
		}

		list.clear();

		updateFunctionList();

		// Hide the function manager for a while
		m_functionManager->hide();

		// Add another step after a delay (to show that the step was added)
		QTimer::singleShot(250, this, SLOT(slotAddAnother()));
	}
	else
	{
		delete m_functionManager;
		m_functionManager = NULL;
	}
}

//
// Add another step until Cancel is clicked
//
void FunctionCollectionEditor::slotAddAnother()
{
	m_functionManager->show();
}

//
// Remove a function
//
void FunctionCollectionEditor::slotRemoveFunctionClicked()
{
  if (m_functionList->selectedItem() != NULL)
    {
      t_function_id id = 0;

      id = m_functionList->selectedItem()->text(2).toInt();

      if (m_fc->removeItem(id))
	{
	  m_functionList->takeItem(m_functionList->selectedItem());
	}
      else
	{
	  ASSERT(false);
	}
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
  assert(m_fc);
  QString device = QString::null;
  QString function = QString::null;

  QValueList<t_function_id>::iterator it;

  m_functionList->clear();

  for (it = m_fc->steps()->begin(); it != m_fc->steps()->end(); ++it)
    {
      Function* f = _app->doc()->function(*it);
      if (!f)
	{
	  function = QString("Invalid");
	  device = QString("Invalid");
	}
      else if (f->device() != KNoID)
	{
	  function = f->name();

	  Device* d = _app->doc()->device(f->device());
	  if (!d)
	    {
	      device = QString("Invalid");
	    }
	  else
	    {
	      device = d->name();
	    }
	}
      else
	{
	  function = f->name();
	  device = QString("Global");
	}

      QString fid;
      fid.setNum(*it);
      new QListViewItem(m_functionList, device, function, fid);
    }
}


//
// Find an item from function list
//
bool FunctionCollectionEditor::isAlreadyMember(t_function_id id)
{
  QValueList<t_function_id>::iterator it;

  for (it = m_fc->steps()->begin(); it != m_fc->steps()->end(); ++it)
    {
      if (*it == id)
	{
	  return true;
	}
    }

  return false;
}
