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
#include "settings.h"
#include "configkeys.h"

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
  m_chaser = new Chaser();
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
  QString device = QString::null;
  QString function = QString::null;

  m_stepList->clear();

  QValueList<t_function_id>::iterator it;
  it = m_chaser->steps()->end();
  for (unsigned int i = 0; i < m_chaser->steps()->count(); i++)
    {
      --it;
      Function* f = _app->doc()->function(*it);
      if (!f)
	{
	  device = QString("Invalid");
	  function = QString("Invalid");
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
      new QListViewItem(m_stepList, "###", device, function, fid);
    }

  updateOrderNumbers();
}

void ChaserEditor::init()
{
  m_nameEdit->setText(m_chaser->name());
  m_nameEdit->setSelection(0, m_nameEdit->text().length());

  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;
  
  m_raiseButton->setIconSet(QPixmap(dir + "/up.xpm"));
  m_lowerButton->setIconSet(QPixmap(dir + "/down.xpm"));

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
      m_chaser->removeStep(item->text(COL_NUM).toInt() - 1);
    }

  updateStepList();
}

void ChaserEditor::slotAddClicked()
{
  FunctionTree* ft = new FunctionTree(this);
  ft->setInactiveID(m_original->id());

  if (ft->exec() == QDialog::Accepted && ft->functionID() != KNoID)
    {
      m_chaser->addStep(ft->functionID());
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
