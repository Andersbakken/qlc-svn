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
#include "device.h"
#include "app.h"
#include "doc.h"
#include "functiontree.h"
#include "bus.h"
#include "chaser.h"
#include "functionstep.h"

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

extern App* _app;

#define COL_NUM      0
#define COL_DEVICE   1
#define COL_FUNCTION 2
#define COL_FID      3

ChaserEditor::ChaserEditor(Chaser* function, QWidget* parent)
  : UI_ChaserEditor(parent, "", true)
{
  m_chaser = new Chaser(KFunctionIDTemp);
  m_chaser->copyFrom(function);
  ASSERT(m_chaser != NULL);

  m_bus = NULL;

  m_original = function;
}

ChaserEditor::~ChaserEditor()
{
  delete m_chaser;
}

void ChaserEditor::updateStepList()
{
  FunctionStep* step = NULL;
  QString device = NULL;
  QString fid;

  m_stepList->clear();

  QPtrList <FunctionStep> *steps = m_chaser->steps();

  for (int i = steps->count() - 1; i >= 0; i--)
    {
      step = steps->at(i);
      ASSERT(step->function() != NULL);

      if (step->function()->device() != NULL)
	{
	  device = step->function()->device()->name();
	}
      else
	{
	  device = QString("Global");
	}

      fid.setNum(step->function()->id());
      new QListViewItem(m_stepList, "###", device,
			step->function()->name(), fid);
    }

  updateOrderNumbers();
}

void ChaserEditor::init()
{
  m_nameEdit->setText(m_chaser->name());
  m_nameEdit->setSelection(0, m_nameEdit->text().length());

  m_reverse->setChecked( (m_chaser->direction() == Chaser::Reverse) );
  m_runOrderGroup->setButton((int) m_chaser->runOrder());

  m_stepList->setSorting(-1);

  updateStepList();
}

void ChaserEditor::slotOKClicked()
{
  m_chaser->setName(m_nameEdit->text());
  m_chaser->setDirection((m_reverse->isChecked()) ? 
			 Chaser::Reverse : Chaser::Normal);
  m_chaser->setRunOrder((Chaser::RunOrder) m_runOrderGroup->selectedId());

  m_original->copyFrom(m_chaser, false);

  _app->doc()->setModified(true);
  accept();
}

void ChaserEditor::slotCancelClicked()
{
  reject();
}

void ChaserEditor::slotRemoveClicked()
{
  QListViewItem* item = m_stepList->currentItem();

  if (item != NULL)
    {
      int index = item->text(COL_NUM).toInt();
      m_chaser->removeStep(index);
    }

  updateStepList();
}

void ChaserEditor::slotAddClicked()
{
  FunctionTree* ft = new FunctionTree(this);

  if (ft->exec() == QDialog::Accepted && ft->functionID() != 0)
    {
      Function* function = NULL;
      function = _app->doc()->searchFunction(ft->functionID());
      ASSERT(function != NULL);

      m_chaser->addStep(function);
    }

  delete ft;

  updateStepList();
}

void ChaserEditor::slotPlayClicked()
{
  qDebug("Not implemented");
}

void ChaserEditor::updateOrderNumbers()
{
  int i = 0;
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

void ChaserEditor::slotRaiseClicked()
{
  QListViewItem* item = m_stepList->currentItem();
  t_function_id fid = 0;
  int index = 0;
  int newIndex = 0;

  if (item != NULL)
    {
      index = item->text(COL_NUM).toInt();
      fid = item->text(COL_FID).toULong();

      m_chaser->raiseStep(index);
    }

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

void ChaserEditor::slotLowerClicked()
{
  QListViewItem* item = m_stepList->currentItem();
  t_function_id fid = 0;
  int index = 0;
  int newIndex = 0;

  if (item != NULL)
    {
      index = item->text(COL_NUM).toInt();
      fid = item->text(COL_FID).toULong();

      m_chaser->lowerStep(index);
    }

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
