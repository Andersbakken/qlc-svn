/*
  Q Light Controller
  newinputdevice.cpp

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

#include "app.h"
#include "doc.h"
#include "plugin.h"
#include "settings.h"
#include "newinputdevice.h"
#include "joystickplugin.h"

#include <qpixmap.h>

extern App* _app;

NewInputDevice::NewInputDevice(QWidget* parent, const char* name) : QDialog (parent, name, true)
{
  initView();
  fetchInputDevices();
}

NewInputDevice::~NewInputDevice()
{

}

void NewInputDevice::initView(void)
{
  setCaption("Add New Input Device");
  setIcon(QPixmap(_app->settings()->pixmapPath() + QString("addinputdevice.xpm")));
  setFixedSize(460, 200);

  m_list = new QListView(this);
  m_list->setGeometry(10, 10, 280, 180);
  m_list->addColumn("Name");
  m_list->addColumn("Type");
  m_list->setColumnWidth(0, (int) (280 * 0.5) - 2);
  m_list->setColumnWidth(1, (int) (280 * 0.5) - 2);
  connect(m_list, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectionChanged(QListViewItem*)));

  m_customNameLabel = new QLabel(this);
  m_customNameLabel->setGeometry(300, 10, 150, 30);
  m_customNameLabel->setText("Custom Name");

  m_customNameEdit = new QLineEdit(this);
  m_customNameEdit->setGeometry(300, 40, 150, 30);
  connect(m_customNameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotCustomTextChanged(const QString &)));

  m_ok = new QPushButton(this);
  m_ok->setGeometry(300, 120, 150, 30);
  m_ok->setText("&OK");
  connect(m_ok, SIGNAL(clicked()), this, SLOT(accept()));

  m_cancel = new QPushButton(this);
  m_cancel->setGeometry(300, 160, 150, 30);
  m_cancel->setText("&Cancel");
  connect(m_cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void NewInputDevice::slotSelectionChanged(QListViewItem* item)
{
  if (item != NULL)
    {
      m_nameValue = item->text(0);
      m_typeValue = item->text(1);
    }
}

void NewInputDevice::slotCustomTextChanged(const QString &text)
{
  m_customNameValue = QString(text);
}

void NewInputDevice::fetchInputDevices()
{
  QList <Plugin> list = _app->doc()->pluginList();

  for (unsigned int i = 0; i < list.count(); i++)
    {
      ASSERT(list.at(i) != NULL);

      if (list.at(i)->type() == Plugin::Joystick)
	{
	  JoystickPlugin* jp = (JoystickPlugin*) list.at(i);
	  ASSERT(jp != NULL);

	  new QListViewItem(m_list, jp->name(), jp->typeString());
	}
    }
}
