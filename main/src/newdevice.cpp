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

#include <qpixmap.h>
#include <qheader.h>

extern App* _app;

NewDevice::NewDevice(QWidget *parent, const char *name) : QDialog(parent, name, true)
{
  m_nameValue = QString("");
  m_addressValue = 1;

  initView();
}

NewDevice::~NewDevice()
{

}

void NewDevice::initView()
{
  resize(400, 250);
  setFixedSize(400, 250);
  setCaption("Add New Output Device");

  m_tree = new QListView(this);
  m_tree->setGeometry(10, 10, 220, 230);
  m_tree->addColumn("Device Classes");
  m_tree->setColumnWidth(0, 215);
  m_tree->setRootIsDecorated(true);
  connect(m_tree, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectionChanged(QListViewItem*)));
  fillTree();

  m_nameLabel = new QLabel(this);
  m_nameLabel->setGeometry(240, 10, 150, 20);
  m_nameLabel->setText("Name");

  m_nameEdit = new QLineEdit(this);
  m_nameEdit->setGeometry(240, 30, 150, 20);
  connect(m_nameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotNameChanged(const QString &)));

  m_addressLabel = new QLabel(this);
  m_addressLabel->setGeometry(240, 60, 150, 20);
  m_addressLabel->setText("Address");

  m_addressSpin = new QSpinBox(this);
  m_addressSpin->setGeometry(240, 80, 150, 20);
  m_addressSpin->setRange(1, 512);
  m_addressSpin->setValue(1);

  m_autoAddress = new QCheckBox(this);
  m_autoAddress->setGeometry(240, 110, 150, 30);
  m_autoAddress->setText("Automatic address");
  m_autoAddress->setChecked(_app->settings()->getAutoAddressAssign());
  slotAutoAddressClicked();
  connect(m_autoAddress, SIGNAL(clicked()), this, SLOT(slotAutoAddressClicked()));

  m_ok = new QPushButton(this);
  m_ok->setGeometry(240, 170, 150, 30);
  m_ok->setText("&OK");
  connect(m_ok, SIGNAL(clicked()), this, SLOT(slotOKClicked()));

  m_cancel = new QPushButton(this);
  m_cancel->setGeometry(240, 210, 150, 30);
  m_cancel->setText("&Cancel");
  connect(m_cancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
}

void NewDevice::slotAutoAddressClicked()
{
  bool on = m_autoAddress->isChecked();
  _app->settings()->setAutoAddressAssign(on);
  m_addressSpin->setEnabled(!on);
}

void NewDevice::slotSelectionChanged(QListViewItem* item)
{
  if (item->parent() != NULL)
    {
      m_modelValue = item->text(0);
      m_manufacturerValue = item->parent()->text(0);

      m_nameEdit->setText(m_manufacturerValue + QString(" ") + m_modelValue);
    }
  else
    {
      m_manufacturerValue = QString("");
      m_modelValue = QString("");
      m_nameEdit->setText(QString(""));
    }
}

void NewDevice::fillTree()
{
  QListViewItem* parent = NULL;
  QListViewItem* newItem = NULL;

  QList <DeviceClass> dclist(_app->doc()->deviceClassList());
  QPixmap pm(_app->settings()->getPixmapPath() + QString("dmx.xpm"));

  m_tree->clear();

  for (DeviceClass* dc = dclist.first(); dc != NULL; dc = dclist.next())
    {
      bool alreadyAdded = false;

      for (QListViewItem* i = m_tree->firstChild(); i != NULL; i = i->nextSibling())
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
	}

      parent->setPixmap(0, QPixmap(_app->settings()->getPixmapPath() + QString("global.xpm")));

      newItem = new QListViewItem(parent, dc->model());
      newItem->setPixmap(0, pm);
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
  m_addressValue = m_addressSpin->value();

  accept();
}

void NewDevice::slotCancelClicked()
{
  reject();
}

void NewDevice::closeEvent(QCloseEvent* e)
{
  slotCancelClicked();
}
