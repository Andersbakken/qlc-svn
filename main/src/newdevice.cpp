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

extern App* _app;

NewDevice::NewDevice(QWidget *parent, const char *name) : QDialog(parent, name, true)
{
  m_nameValue = QString("");
  m_addressValue = 1;

  init();
}

NewDevice::~NewDevice()
{

}

void NewDevice::init()
{
  resize(340, 250);
  setFixedSize(340, 250);
  setCaption("Add New Device");

  m_qlabel1 = new QLabel(this);
  m_qlabel1->setGeometry(10, 10, 130, 30);
  m_qlabel1->setText("Manufacturer");

  m_manufacturer = new QComboBox(this);
  m_manufacturer->setGeometry(130, 10, 200, 30);
  connect(m_manufacturer, SIGNAL(activated(int)), this, SLOT(slotManufacturerActivated(int)));

  m_qlabel2 = new QLabel(this);
  m_qlabel2->setGeometry(10, 50, 130, 30);
  m_qlabel2->setText("Model");

  m_model = new QComboBox(this);
  m_model->setGeometry(130, 50, 200, 30);
  connect(m_model, SIGNAL(activated(int)), this, SLOT(slotModelActivated(int)));

  m_qlabel3 = new QLabel(this);
  m_qlabel3->setGeometry(10, 90, 130, 30);
  m_qlabel3->setText("Name");

  m_name = new QLineEdit(this);
  m_name->setGeometry(130, 90, 200, 30);
  connect(m_name, SIGNAL(textChanged(const QString &)), this, SLOT(slotNameChanged(const QString &)));

  m_qlabel4 = new QLabel(this);
  m_qlabel4->setGeometry(10, 130, 130, 30);
  m_qlabel4->setText("Address");

  m_address = new QSpinBox(this);
  m_address->setGeometry(130, 130, 200, 30);
  m_address->setRange(1, 512);
  m_address->setValue(1);

  m_autoAddress = new QCheckBox(this);
  m_autoAddress->setGeometry(130, 170, 200, 30);
  m_autoAddress->setText("Automatic address");
  m_autoAddress->setChecked(_app->settings()->getAutoAddressAssign());
  slotAutoAddressClicked();
  connect(m_autoAddress, SIGNAL(clicked()), this, SLOT(slotAutoAddressClicked()));

  showManufacturers();

  m_ok = new QPushButton(this);
  m_ok->setGeometry(130, 210, 100, 30);
  m_ok->setText("&OK");
  connect(m_ok, SIGNAL(clicked()), this, SLOT(slotOKClicked()));

  m_cancel = new QPushButton(this);
  m_cancel->setGeometry(230, 210, 100, 30);
  m_cancel->setText("&Cancel");
  connect(m_cancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
}

void NewDevice::slotAutoAddressClicked()
{
  bool on = m_autoAddress->isChecked();
  _app->settings()->setAutoAddressAssign(on);
  m_address->setEnabled(!on);
}

void NewDevice::showManufacturers()
{
  m_manufacturer->clear();

  QList <DeviceClass> dclist(_app->doc()->deviceClassList());
  for (DeviceClass* dc = dclist.first(); dc != NULL; dc = dclist.next())
    {
      QPixmap pm(_app->settings()->getPixmapPath() + QString("dmx.xpm"));
      bool alreadyAdded = false;
      for (int i = 0; i < m_manufacturer->count(); i++)
	{
	  alreadyAdded = false;
	  
	  if (m_manufacturer->text(i) == dc->manufacturer())
	    {
	      alreadyAdded = true;
	      break;
	    }
	}
      
      if (alreadyAdded == false)
	{
	  m_manufacturer->insertItem(pm, dc->manufacturer());
	}
    }

  slotManufacturerActivated(m_manufacturer->currentItem());
}

void NewDevice::showModels(QString mfg)
{
  m_model->clear();

  QList <DeviceClass> dclist(_app->doc()->deviceClassList());
  for (DeviceClass* dc = dclist.first(); dc != NULL; dc = dclist.next())
    {
      if (mfg == dc->manufacturer())
	{
	  if (dc->protocol() == DeviceClass::DMX512)
	    {
	      QPixmap pm(_app->settings()->getPixmapPath() + QString("dmx.xpm"));
	      m_model->insertItem(pm, dc->model());
	    }
	  else
	    {
	      m_model->insertItem(dc->model());
	    }
	}
    }

  slotModelActivated(m_model->currentItem());
}

void NewDevice::slotNameChanged(const QString &text)
{

}

void NewDevice::slotManufacturerActivated(int item)
{
  showModels(m_manufacturer->currentText());
}

void NewDevice::slotModelActivated(int item)
{
  m_name->setText(m_manufacturer->currentText() + " " + m_model->currentText());
}

void NewDevice::show()
{
  QDialog::show();
}

void NewDevice::slotOKClicked()
{
  m_nameValue = m_name->text();
  m_addressValue = m_address->value();
  m_manufacturerValue = m_manufacturer->currentText();
  m_modelValue = m_model->currentText();

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
