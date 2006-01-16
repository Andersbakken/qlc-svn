/*
  Q Light Controller
  vcbuttonproperties.h

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

#ifndef VCBUTTONPROPERTIES_H
#define VCBUTTONPROPERTIES_H

#include "types.h"
#include "vcbutton.h"
#include "uic_vcbuttonproperties.h"

class FunctionManager;
class KeyBind;

class VCButtonProperties : public UI_VCButtonProperties
{
  Q_OBJECT

 public:
  VCButtonProperties(VCButton*, QWidget* parent = 0, const char* name = 0);
  ~VCButtonProperties();

  void initView();

 public slots:
  void slotPressGroupClicked(int);

  void slotAttachFunctionClicked();
  void slotDetachFunctionClicked();

  void slotAttachKeyClicked();
  void slotFunctionManagerClosed();
  void slotDetachKeyClicked();

  void slotOKClicked();
  void slotCancelClicked();

 private:
  void setFunctionName();

 private:
  VCButton* m_button;
  KeyBind* m_keyBind;
  t_function_id m_functionID;
  FunctionManager* m_functionManager;
};

#endif
