/*
  Q Light Controller
  sceneeditor.cpp

  Copyright (C) 2000, 2001, 2002 Heikki Junnila
                            2002 Stefan Krumm

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


#include "sceneeditor.h"
#include "function.h"
#include "deviceclass.h"

#include <qcombobox.h>


SceneEditor::SceneEditor(Device* device, QWidget* parent, const char* name )
             : UI_SceneEditor( parent, name)
{
  Function* f;
  QList <Function> fl = device->deviceClass()->functions();

  for (f = fl.first(); f != NULL; f = fl.next())
    {
      if( f->typeString() == "Scene")
           m_availableScenesComboBox->insertItem(f->name());
    }
}


SceneEditor::~SceneEditor()
{

}

