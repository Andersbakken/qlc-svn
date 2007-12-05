/*
  Q Light Controller
  functioncollectioneditor.cpp

  Copyright (c) Heikki Junnila

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

#include "common/qlcfixturedef.h"

#include "functioncollectioneditor.h"
#include "functioncollection.h"
#include "function.h"
#include "functionmanager.h"
#include "app.h"
#include "doc.h"
#include "fixture.h"

extern App* _app;

FunctionCollectionEditor::FunctionCollectionEditor(QWidget* parent,
						   FunctionCollection* fc)
	: UI_FunctionCollectionEditor(parent,
				      "Function Collection Editor",
				      true)
{
	Q_ASSERT(fc != NULL);

	m_fc = new FunctionCollection();
	m_fc->copyFrom(fc);
	Q_ASSERT(m_fc != NULL);

	m_original = fc;
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
	m_addFunction->setPixmap(QPixmap(QString(PIXMAPS) +
					 QString("/edit_add.png")));
	m_removeFunction->setPixmap(QPixmap(QString(PIXMAPS) +
					    QString("/edit_remove.png")));

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
		m_functionManager = new FunctionManager(
			this, FunctionManager::SelectionMode);

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

	_app->doc()->setModified();

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
	QString fxi_name;
	QString func_name;
	QString func_type;
	Fixture* fxi = NULL;
	Function* function = NULL;
	QValueList<t_function_id>::iterator it;
	QString fid;

	m_functionList->clear();

	for (it = m_fc->steps()->begin(); it != m_fc->steps()->end(); ++it)
	{
		function = _app->doc()->function(*it);
		if (function == NULL)
		{
			func_name = QString("Invalid");
			fxi_name = QString("Invalid");
			func_type = QString("Invalid");
		}
		else if (function->fixture() != KNoID)
		{
			func_name = function->name();
			func_type = Function::typeToString(function->type());

			fxi = _app->doc()->fixture(function->fixture());
			if (fxi == NULL)
				fxi_name = QString("Invalid");
			else
				fxi_name = fxi->name();
		}
		else
		{
			fxi_name = QString("Global");
			func_name = function->name();
			func_type = Function::typeToString(function->type());
		}

		fid.setNum(*it);
		new QListViewItem(m_functionList, fxi_name, func_name,
				  func_type, fid);
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
