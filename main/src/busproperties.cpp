/*
  Q Light Controller
  busproperties.cpp
  
  Copyright (C) Heikki Junnila
  
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

#include <qwidget.h>
#include <qevent.h>
#include <qlistview.h>
#include <qstring.h>

#include "bus.h"
#include "types.h"
#include "busproperties.h"

const int KColumnID                   ( 0 );
const int KColumnName                 ( 1 );

BusProperties::BusProperties(QWidget* parent, const char* name)
  : UI_BusProperties(parent, name)
{
  fillTree();
}

BusProperties::~BusProperties()
{
}

void BusProperties::slotItemClicked(QListViewItem* item, const QPoint& point,
				    int col)
{
  if (col == KColumnName)
    {
      item->startRename(col);
    }
}

void BusProperties::slotItemRenamed(QListViewItem* item, int col, 
				    const QString &text)
{
  Bus::setName(item->text(KColumnID).toInt() - 1, text);
}

void BusProperties::closeEvent(QCloseEvent* e)
{
  emit closed();
}

void BusProperties::fillTree()
{
  QListViewItem* item = NULL;
  QString text;

  for (t_bus_id i = KBusIDMin; i < KBusCount; i++)
    {
      item = new QListViewItem(m_list);
      text.sprintf("%.2d", i + 1);
      item->setText(KColumnID, text);
      item->setText(KColumnName, Bus::name(i));

      item->setRenameEnabled(KColumnName, true);
    }
}
