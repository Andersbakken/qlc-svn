/*
  Q Light Controller
  listboxiditem.cpp

  Copyright (C) 2003 Heikki Junnila

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

#include "listboxiditem.h"

#include <qpainter.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qrect.h>
#include <qnamespace.h>

ListBoxIDItem::ListBoxIDItem()
  : QListBoxItem()
{
  m_rtti = 0;
  setCustomHighlighting(false);
}

ListBoxIDItem::~ListBoxIDItem()
{
}

int ListBoxIDItem::height(const QListBox* lb) const
{
  return QApplication::fontMetrics().height();
}

void ListBoxIDItem::setRtti(int rtti)
{
  m_rtti = rtti;
}

int ListBoxIDItem::rtti()
{
  return m_rtti;
}

void ListBoxIDItem::setText(const QString &text)
{
  QListBoxItem::setText(text);
}

void ListBoxIDItem::paint(QPainter* p)
{
  QRect rect;

  rect.setLeft(0);
  rect.setRight(listBox()->width());
  rect.setTop(0);
  rect.setBottom(QApplication::fontMetrics().height());

  p->drawText(rect, Qt::AlignLeft, text());
}
