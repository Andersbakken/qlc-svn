/*
  Q Light Controller
  editpresetvalue.cpp

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

#include <qwidget.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include "editpresetvalue.h"


EditPresetValue::EditPresetValue(QWidget* parent, const char* name)
  : UI_EditPresetValue(parent, name, true)
{

}

EditPresetValue::~EditPresetValue()
{
}
