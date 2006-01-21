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

#ifndef ASSIGNSLIDERHOTKEY_H
#define ASSIGNSLIDERHOTKEY_H

#include "uic_assignsliderhotkey.h"

class SliderKeyBind;

class AssignSliderHotKey : public UI_AssignSliderHotKey
{
  Q_OBJECT

 public:
  AssignSliderHotKey(QWidget* parent = NULL, const char* name = NULL);
  ~AssignSliderHotKey();

  SliderKeyBind* sliderKeyBind() { return m_sliderKeyBind; }

  void initView();

 public slots:
  void slotOKClicked();
  void slotCancelClicked();

 protected:
  void keyPressEvent(QKeyEvent*);
//  void keyPressEventDown(QKeyEvent*);

 private:
  SliderKeyBind* m_sliderKeyBind;
};

#endif
