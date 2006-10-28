/*
  Q Light Controller
  newdevice.cpp

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

#include "newdevice.h"
#include "deviceclass.h"
#include "common/settings.h"
#include "app.h"
#include "doc.h"
#include "dmxaddresstool.h"
#include "configkeys.h"

#include <qpixmap.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qevent.h>
#include <qcheckbox.h>
#include <assert.h>
#include <qmessagebox.h>

extern App* _app;

//
// Keys for settings
//
const QString KEY_NEWDEVICE_TREE_OPEN = "NewDeviceTreeOpen";

//
// Constructor
//
NewDevice::NewDevice(QWidget *parent, const char *name)
	: UI_NewDevice(parent, name, true),
	
	m_addressValue        ( 0 ),
	m_universeValue       ( 0 ),
	m_multipleNumberValue ( 1 ),
	m_addressGapValue     ( 0 ),
	m_selectionOK         ( false )
{
}

//
// Destructor
//
NewDevice::~NewDevice()
{

}

//
// Initialize view components
//
void NewDevice::initView()
{
  QString config;

  // Default range for address spin
  m_addressSpin->setRange(1, 512);

  // Range for universe spin
  m_universeSpin->setRange(1, KUniverseCount);

  fillTree();

  if (_app->settings()->get(KEY_NEWDEVICE_TREE_OPEN, config) != -1
	&& config == Settings::trueValue())
  {
	m_treeOpenCheckBox->setChecked(true);
  }
  else
  {
	m_treeOpenCheckBox->setChecked(false);
  }

  m_ok->setEnabled(false);
}

//
// Set address with mockup DIP switches
//
void NewDevice::slotDIPClicked()
{
  DMXAddressTool* dat = new DMXAddressTool(_app);
  dat->setAddress(m_addressSpin->value());
  if (dat->exec() == QDialog::Accepted)
    {
      if (dat->address() > 0)
	{
	  m_addressSpin->setValue(dat->address());
	}
      else
	{
	  m_addressSpin->setValue(1);
	}
    }

  delete dat;
}

//
// Open/close all manufacturer nodes
//
void NewDevice::slotTreeOpenCheckBoxClicked()
{
  if (m_treeOpenCheckBox->isChecked() == true)
    {
      _app->settings()->set(KEY_NEWDEVICE_TREE_OPEN, Settings::trueValue());

      /* Open all folders */
      QListViewItemIterator it(m_tree);
      while (it.current())
	{
	  it.current()->setOpen(true);
	  it++;
	}
    }
  else
    {
      _app->settings()->set(KEY_NEWDEVICE_TREE_OPEN, Settings::falseValue());

      /* Close all folders */
      QListViewItemIterator it(m_tree);
      while (it.current())
	{
	  it.current()->setOpen(false);
	  it++;
	}
    }
}

//
// An item has been doubleclicked, same thing as pressing OK
//
void NewDevice::slotTreeDoubleClicked(QListViewItem* item)
{
	slotSelectionChanged(item);

	slotOKClicked();
}

//
// Selection has changed in the device tree view
//
void NewDevice::slotSelectionChanged(QListViewItem* item)
{
  if (item->parent() != NULL)
    {
      m_selectionOK = true;
      m_modelValue = item->text(0);
      m_manufacturerValue = item->parent()->text(0);
      if (m_nameEdit->isModified() == false)
	{
	  /* Set the name to the model name ONLY if the user hasn't
	     modified the name field. */
	  m_nameEdit->setText(m_modelValue);
	}

      m_nameEdit->setSelection(0, m_nameEdit->text().length());
      m_nameEdit->setFocus();

      DeviceClass* dc = _app->searchDeviceClass(m_manufacturerValue,
						m_modelValue);
      assert(dc);
      m_addressSpin->setRange(1, 513 - dc->channels()->count());
      m_channelsSpin->setValue(dc->channels()->count());
      m_ok->setEnabled(true);
    }
  else
    {
      m_ok->setEnabled(false);
      m_selectionOK = false;
      m_manufacturerValue = QString("");
      m_modelValue = QString("");
      if (m_nameEdit->isModified() == false)
	{
		m_nameEdit->setText(QString::null);
	}

      m_channelsSpin->setValue(0);
    }
}

//
// Fill all known deviceclasses to the tree
//
void NewDevice::fillTree()
{
  QListViewItem* parent = NULL;
  QListViewItem* newItem = NULL;

  QPtrList <DeviceClass> *dl = _app->deviceClassList();

  QString config;
  bool treeOpen = false;
  if (_app->settings()->get(KEY_NEWDEVICE_TREE_OPEN, config) != -1 &&
	config == Settings::trueValue())
  {
	treeOpen = true;
  }
  else
  {
	treeOpen = false;
  }

  m_tree->clear();

  /* Add all known device classes */
  for (DeviceClass* dc = dl->first(); dc != NULL; dc = dl->next())
    {
      bool alreadyAdded = false;

      for (QListViewItem* i = m_tree->firstChild();
	   i != NULL; i = i->nextSibling())
	{
	  if (i->text(0) == dc->manufacturer())
	    {
	      alreadyAdded = true;
	      parent = i;
	      break;
	    }
	}

      if (alreadyAdded == false)
	{
	  parent = new QListViewItem(m_tree, dc->manufacturer());
	  parent->setOpen(treeOpen);
	}

      newItem = new QListViewItem(parent, dc->model());
      newItem->setText(1,dc->type());
    }
}

//
// User has edited the name field
//
void NewDevice::slotNameChanged(const QString &text)
{
	m_nameValue = text;
	if (text.length() == 0)
	{
		/* If the user clears the text in the name field,
		   start substituting the name with the model again. */
		m_nameEdit->clearModified();
	}
}

//
// Show the dialog
//
void NewDevice::show()
{
	QDialog::show();
}

//
// OK button was clicked
//
void NewDevice::slotOKClicked()
{
	m_addressValue = m_addressSpin->value() - 1;
	m_universeValue = m_universeSpin->value() - 1;
	
	m_multipleNumberValue = m_multipleNumberSpin->value();
	m_addressGapValue = m_addressGapSpin->value();
	
	if (m_selectionOK == true)
	{
		accept();
	}
}

//
// Cancel was clicked
//
void NewDevice::slotCancelClicked()
{
	reject();
}
