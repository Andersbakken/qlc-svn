/*
  Q Light Controller
  assignhotkey.h
  
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

#ifndef ASSIGNHOTKEY_H
#define ASSIGNHOTKEY_H

#include "classes.h"
#include "uic_assignhotkey.h"

class AssignHotKey : public UI_AssignHotKey
{
  Q_OBJECT

 public:
  AssignHotKey(QWidget* parent = NULL, const char* name = NULL);
  ~AssignHotKey();

  const KeyBind* keyBind() { return (const KeyBind*) m_keyBind; }

  void initView();

 public slots:
  void slotOKClicked();
  void slotCancelClicked();

 protected:
  void keyPressEvent(QKeyEvent*);

 private:
  KeyBind* m_keyBind;
};

#endif
