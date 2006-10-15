/*
  Q Light Controller
  listboxiditem.h

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

#ifndef LISTBOXIDITEM_H
#define LISTBOXIDITEM_H

#include <qlistbox.h>

class ListBoxIDItem : public QListBoxItem
{
 public:
  ListBoxIDItem();
  virtual ~ListBoxIDItem();

  void setRtti(int rtti);
  virtual int rtti();

  void setText(const QString & text);

 protected:
  virtual int width(const QListBox* lb) const;
  virtual int height(const QListBox*) const;
  virtual void paint(QPainter* p);

 protected:
  int m_rtti;
  QString m_text;
};

#endif
