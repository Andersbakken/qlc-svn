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
#include <qlabel.h>
#include <qinputdialog.h>


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
  
  m_statusLabel->setPaletteForegroundColor( QColor( 0, 255, 100 ) );
  m_statusLabel->setText("unchanged");

}


SceneEditor::~SceneEditor()
{

}



void SceneEditor::slotSceneChanged()
{
   m_statusLabel->setPaletteForegroundColor( QColor( 255, 0, 0 ) );
   m_statusLabel->setText("modified");
}


void SceneEditor::slotSceneActivated( int nr )
{
  m_statusLabel->setPaletteForegroundColor( QColor( 0, 255, 100 ) );
  m_statusLabel->setText("unchanged");
}

void  SceneEditor::slotHideClicked()
{

}


void  SceneEditor::slotNewClicked()
{
 bool ok = FALSE;
  QString text = QInputDialog::getText(
                    tr( "Scene editor" ),
                    tr( "Please enter scene name" ),
                    QLineEdit::Normal, QString::null, &ok, this );
  if ( ok && !text.isEmpty() )
  {
          m_availableScenesComboBox->insertItem( text );
          m_statusLabel->setPaletteForegroundColor( QColor( 0, 255, 100 ) );
          m_statusLabel->setText("unchanged");
  }
  else
     slotNewClicked();
}

void  SceneEditor::slotSaveClicked()
{
  m_statusLabel->setPaletteForegroundColor( QColor( 0, 0, 255 ) );
  m_statusLabel->setText("saved");
}