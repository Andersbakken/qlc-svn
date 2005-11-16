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
#include "settings.h"
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

extern App* _app;

// Keys for settings
const QString KEY_NEWDEVICE_TREE_OPEN = "NewDeviceTreeOpen";

NewDevice::NewDevice(QWidget *parent, const char *name) 
  : UI_NewDevice(parent, name, true)
{
  m_nameValue = QString("");
  m_addressValue = 0;
  m_selectionOK = false;

  initView();
}

NewDevice::~NewDevice()
{

}

void NewDevice::initView()
{
  QString config;

  // Default range for address spin
  m_addressSpin->setRange(1, 512);

  // Range for universe spin
  m_universeSpin->setRange(1, KUniverseCount);

  fillTree();

  _app->settings()->get(KEY_NEWDEVICE_TREE_OPEN, config);
  m_treeOpenCheckBox->setChecked( (config == Settings::trueValue()) ? 
				  true : false );

  m_nameEdit->setText("New device");
  m_ok->setEnabled(false);
}

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

void NewDevice::slotTreeOpenCheckBoxClicked()
{
  if (m_treeOpenCheckBox->isChecked() == true)
    {
      _app->settings()->set(KEY_NEWDEVICE_TREE_OPEN, Settings::trueValue());
    }
  else
    {
      _app->settings()->set(KEY_NEWDEVICE_TREE_OPEN, Settings::falseValue());
    }
}

void NewDevice::slotTreeDoubleClicked(QListViewItem* item)
{
  slotSelectionChanged(item);

  slotOKClicked();
}

void NewDevice::slotSelectionChanged(QListViewItem* item)
{
  if (item->parent() != NULL)
    {
      m_selectionOK = true;
      m_modelValue = item->text(0);
      m_manufacturerValue = item->parent()->text(0);
      if (m_nameEdit->text() == QString::null)
	{
	  m_nameEdit->setText("New device");
	}

      m_nameEdit->setSelection(0, m_nameEdit->text().length());
      m_nameEdit->setFocus();

      DeviceClass* dc = _app->searchDeviceClass(m_manufacturerValue,
						m_modelValue);
      assert(dc);
      m_addressSpin->setRange(1, 513 - dc->channels()->count());
      m_ok->setEnabled(true);
    }
  else
    {
      m_ok->setEnabled(false);
      m_selectionOK = false;
      m_manufacturerValue = QString("");
      m_modelValue = QString("");
      m_nameEdit->setText(QString(""));
    }
}

void NewDevice::fillTree()
{
  QListViewItem* parent = NULL;
  QListViewItem* newItem = NULL;

  QPtrList <DeviceClass> *dl = _app->deviceClassList();

  QString path;
  _app->settings()->get("SystemPath", path);
  path += QString("/") + PIXMAPPATH;
  QPixmap pm(path + QString("/dmx.xpm"));

  QString tree;
  _app->settings()->get("NewDeviceTreeOpen", tree);
  bool treeOpen = (tree == Settings::trueValue()) ? true : false;

  m_tree->clear();

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

      parent->setPixmap(0, QPixmap(path + QString("/global.xpm")));

      newItem = new QListViewItem(parent, dc->model());
      newItem->setPixmap(0, pm);
      newItem->setText(1,dc->type());
    }
}

void NewDevice::slotNameChanged(const QString &text)
{
  m_nameValue = text;
}

void NewDevice::show()
{
  QDialog::show();
}

void NewDevice::slotOKClicked()
{
  m_addressValue = m_addressSpin->value() - 1;

  m_universeValue = m_universeSpin->value() - 1;

  if (m_selectionOK == true)
    {
      accept();
    }
}

void NewDevice::slotCancelClicked()
{
  reject();
}
