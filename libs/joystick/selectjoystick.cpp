/*
  Q Light Controller
  selectjoystick.cpp

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

#include <qlistview.h>

#include "selectjoystick.h"
#include "joystickplugin.h"
#include "joystick.h"

SelectJoystick::SelectJoystick(JoystickPlugin* plugin)
  : UI_SelectJoystick(NULL, "Select Joystick", true)
{
  m_plugin = plugin;
  m_currentJoystick = NULL;
}

SelectJoystick::~SelectJoystick()
{

}

void SelectJoystick::initView()
{
  m_joystickList->setMultiSelection(false);
  m_joystickList->setAllColumnsShowFocus(true);
  m_joystickList->setSorting(3, true);
  m_joystickList->setColumnAlignment(1, AlignHCenter);
  m_joystickList->setColumnAlignment(2, AlignHCenter);

  int w = m_joystickList->width();

  m_joystickList->setColumnWidth(0, (int) (w * 0.45 - 0.5));
  m_joystickList->setColumnWidth(1, (int) (w * 0.15 - 0.5));
  m_joystickList->setColumnWidth(2, (int) (w * 0.15 - 0.5));
  m_joystickList->setColumnWidth(3, (int) (w * 0.25 - 0.5));

  fillList();
}

void SelectJoystick::fillList()
{
  QPtrList <Joystick> *list = m_plugin->joystickList();

  if (list->count() == 0)
    {
      m_plugin->open();
    }

  for (Joystick* j = list->first(); j != NULL; j = list->next())
    {
      QString t;
      QListViewItem* item = new QListViewItem(m_joystickList);
      item->setText(0, j->name());
      t.setNum(j->axesList().count());
      item->setText(1, t);
      t.setNum(j->buttonsList().count());
      item->setText(2, t);
      item->setText(3, j->fdName());
    }
}

void SelectJoystick::slotJoystickSelected(QListViewItem* item)
{
  if (item != NULL)
    {
      QString device = item->text(3);
      m_currentJoystick = m_plugin->search(device);

      ASSERT(m_currentJoystick != NULL);
    }
  else
    {
      m_currentJoystick = NULL;
    }
}

void SelectJoystick::slotOKClicked()
{
  accept();
}

void SelectJoystick::slotCancelClicked()
{
  reject();
}

void SelectJoystick::slotCalibrateClicked()
{
}
