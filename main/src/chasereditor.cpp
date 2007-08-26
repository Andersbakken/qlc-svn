/*
  Q Light Controller
  chasereditor.cpp

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

#include <stdlib.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <limits.h>
#include <qmessagebox.h>
#include <assert.h>
#include <qtimer.h>

#include "common/qlcfixturedef.h"
#include "common/settings.h"

#include "chasereditor.h"
#include "function.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"
#include "functionmanager.h"
#include "bus.h"
#include "chaser.h"
#include "configkeys.h"

extern App* _app;

#define COL_NUM      0
#define COL_FIXTURE  1
#define COL_FUNCTION 2
#define COL_FID      3

//
// Constructor
//
ChaserEditor::ChaserEditor(Chaser* function, QWidget* parent)
	: UI_ChaserEditor(parent, "ChaserEditor", true)
{
	m_chaser = new Chaser();
	m_chaser->copyFrom(function);

	Q_ASSERT(m_chaser != NULL);

	m_bus = NULL;

	m_original = function;

	m_functionManager = NULL;
}

//
// Destructor
//
ChaserEditor::~ChaserEditor()
{
	delete m_chaser;
}

//
// Insert chaser steps into the editor's list
//
void ChaserEditor::updateStepList()
{
	QString fxi_name = QString::null;
	QString func_name = QString::null;
	Fixture* fxi = NULL;

	m_stepList->clear();

	QValueList<t_function_id>::iterator it;
	it = m_chaser->steps()->end();
	for (unsigned int i = 0; i < m_chaser->steps()->count(); i++)
	{
		--it;
		Function* f = _app->doc()->function(*it);
		if (f == NULL)
		{
			fxi_name = QString("Invalid");
			func_name = QString("Invalid");
		}
		else if (f->fixture() != KNoID)
		{
			func_name = f->name();

			fxi = _app->doc()->fixture(f->fixture());
			if (fxi == NULL)
			{
				fxi_name = QString("Invalid");
			}
			else
			{
				fxi_name = fxi->name();
			}
		}
		else
		{
			func_name = f->name();
			fxi_name = QString("Global");
		}

		QString fid;
		fid.setNum(*it);
		new QListViewItem(m_stepList, "###", fxi_name, func_name, fid);
	}

	updateOrderNumbers();
}

//
// Initialize the UI
//
void ChaserEditor::init()
{
	m_nameEdit->setText(m_chaser->name());
	m_nameEdit->setSelection(0, m_nameEdit->text().length());

	m_addStep->setPixmap(QPixmap(QString(PIXMAPS) + QString("/edit_add.png")));
	m_removeStep->setPixmap(QPixmap(QString(PIXMAPS) + QString("/edit_remove.png")));

	m_raiseButton->setPixmap(QPixmap(QString(PIXMAPS) + QString("/up.png")));
	m_lowerButton->setPixmap(QPixmap(QString(PIXMAPS) + QString("/down.png")));

	m_runOrderGroup->setButton((int) m_chaser->runOrder());
	m_directionGroup->setButton((int) m_chaser->direction());

	m_stepList->setSorting(-1);

	updateStepList();
}

//
// Accept changes and exit
//
void ChaserEditor::slotOKClicked()
{
	//
	// Name
	//
	m_chaser->setName(m_nameEdit->text());

	//
	// Run Order
	//
	if (m_runOrderGroup->selected() == m_singleShot)
	{
		m_chaser->setRunOrder(Chaser::SingleShot);
	}
	else if (m_runOrderGroup->selected() == m_pingPong)
	{
		m_chaser->setRunOrder(Chaser::PingPong);
	}
	else
	{
		m_chaser->setRunOrder(Chaser::Loop);
	}

	//
	// Direction
	//
	if (m_directionGroup->selected() == m_backward)
	{
		m_chaser->setDirection(Chaser::Backward);
	}
	else
	{
		m_chaser->setDirection(Chaser::Forward);
	}

	m_original->copyFrom(m_chaser, false);

	_app->doc()->setModified();
	accept();
}

//
// Cancel editing
//
void ChaserEditor::slotCancelClicked()
{
	reject();
}

//
// Remove the selected step
//
void ChaserEditor::slotRemoveClicked()
{
	QListViewItem* item = m_stepList->currentItem();

	if (item != NULL)
	{
		m_chaser->removeStep(item->text(COL_NUM).toInt() - 1);
	}

	updateStepList();
}

//
// Add a step to the chaser function
//
void ChaserEditor::slotAddClicked()
{
	if (m_functionManager == NULL)
	{
		// Create a new function manager
		m_functionManager = new FunctionManager(this,
							FunctionManager::SelectionMode);

		connect(m_functionManager, SIGNAL(closed()),
			this, SLOT(slotFunctionManagerClosed()));
		// Prevent the user from selecting this function
		m_functionManager->setInactiveID(m_original->id());

		// Initialize the function manager UI
		m_functionManager->init();
	}
	
	m_functionManager->show();
}

//
// Function manager was closed
//
void ChaserEditor::slotFunctionManagerClosed()
{
	FunctionIDList list;
	FunctionIDList::iterator it;
	Function* function = NULL;

	assert(m_functionManager);

	if (m_functionManager->result() == QDialog::Accepted)
	{
		m_functionManager->selection(list);

		for (it = list.begin(); it != list.end(); ++it)
		{
			m_chaser->addStep(*it);
		}

		// Clear the selection list
		list.clear();

		// Update steps to the list
		updateStepList();
		
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
void ChaserEditor::slotAddAnother()
{
	m_functionManager->show();
}

//
// Test run
//
void ChaserEditor::slotPlayClicked()
{
	qDebug("Not implemented");
}

//
// Update correct order numbers to each step
//
void ChaserEditor::updateOrderNumbers()
{
	int i = 1;
	QString size;
	QString num;

	size.setNum(m_chaser->steps()->count());

	// Create an iterator and give the listview as argument
	QListViewItemIterator it(m_stepList);

	// Iterate through all items of the listview
	for (; it.current() != NULL; ++it)
	{
		num.sprintf("%.03d", i++);
		it.current()->setText(COL_NUM, num);
	}
}

//
// Raise a chaser step one position higher
//
void ChaserEditor::slotRaiseClicked()
{
	QListViewItem* item = m_stepList->currentItem();
	t_function_id fid = 0;
	int index = 0;
	int newIndex = 0;

	if (item != NULL)
	{
		index = item->text(COL_NUM).toInt() - 1;
		fid = item->text(COL_FID).toInt();

		if (m_chaser->raiseStep(index))
		{
			updateStepList();

			// Select the item again, now it's one step above
			QListViewItemIterator it(m_stepList);
			while (it.current() != NULL)
			{
				if (newIndex == index - 1)
				{
					m_stepList->setSelected(it.current(), true);
					break;
				}

				newIndex++;
				++it;
			}
		}
	}
}

//
// Lower a chaser step one position lower
//
void ChaserEditor::slotLowerClicked()
{
	QListViewItem* item = m_stepList->currentItem();
	t_function_id fid = 0;
	int index = 0;
	int newIndex = 0;

	if (item != NULL)
	{
		index = item->text(COL_NUM).toInt() - 1;
		fid = item->text(COL_FID).toInt();

		if (m_chaser->lowerStep(index))
		{
			updateStepList();

			// Select the item again, now it's one step below
			QListViewItemIterator it(m_stepList);
			while (it.current() != NULL)
			{
				if (newIndex == index + 1)
				{
					m_stepList->setSelected(it.current(), true);
					break;
				}

				newIndex++;
				++it;
			}
		}
	}
}
