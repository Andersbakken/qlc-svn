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

#include "vcbutton.h"
#include "uic_vcbuttonproperties.h"

class VCButtonProperties : public UI_VCButtonProperties
{
  Q_OBJECT

 public:
  VCButtonProperties(VCButton* btn, QWidget* parent = 0, const char* name = 0);
  ~VCButtonProperties();
  
  VCButton* button() { return m_button; }

  Function* function() { return m_function; }
  QString name() { return m_nameString; }
  KeyBind* keyBind() { return m_keyBind; }
  bool lock() { return m_lockState; }
  Bus* bus() { return m_bus; }
  
 public slots:
  void slotPressGroupClicked(int);
  void slotReleaseGroupClicked(int);
  void slotOKClicked();
  void slotCancelClicked();
  void slotAddFunctionClicked();
  void slotRemoveFunctionClicked();
  void slotAddKeyClicked();
  void slotRemoveKeyClicked();
  void slotAddBusClicked();
  void slotBusActivated(int);

 private:
  void initView();
  void fillBusCombo();
  void selectCurrentBus();

 private:
  VCButton* m_button;

  Function* m_function;

  QString m_nameString;

  KeyBind* m_keyBind;

  bool m_lockState;

  Bus* m_bus;
  t_bus_id* m_busIndex;
};

#endif
