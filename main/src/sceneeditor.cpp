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
#include "dmxdevice.h"
#include "scene.h"

#include <qcombobox.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <iostream.h>


SceneEditor::SceneEditor(Device* device, QWidget* parent, const char* name )
             : UI_SceneEditor( parent, name)
{
  m_device = device;
  m_currentScene = NULL;

  // initially, get data from the device class as default.
  m_deviceSource="DeviceClass";
  m_deviceClassRadio->setChecked(TRUE);
  m_selectFunctions(device->deviceClass()->functions());
}


SceneEditor::~SceneEditor()
{

}



void SceneEditor::slotSceneChanged()
{
   m_setStatusText( "modified", QColor( 255, 0, 0 ) );
}


void SceneEditor::slotSceneActivated( int nr )
{
  Function* f;
  if( m_deviceSource == "DeviceClass")
  {
     QList <Function> fl = m_device->deviceClass()->functions();
     for (f = fl.first(); f != NULL; f = fl.next())
     {
        if( f->name() == m_availableScenesComboBox->currentText())
        {
           static_cast<DMXDevice*>(m_device)->setScene(static_cast<Scene*>(f));
	   m_currentScene = static_cast<Scene*>(f);
        }
      }
   }else{
     QList <Function> fl = m_device->functions();
     for (f = fl.first(); f != NULL; f = fl.next())
     {
        if( f->name() == m_availableScenesComboBox->currentText())
        {
           static_cast<DMXDevice*>(m_device)->setScene(static_cast<Scene*>(f));
	   m_currentScene = static_cast<Scene*>(f);
        }
      }
   }

  m_setStatusText( "unchanged", QColor( 0, 255, 100 ));

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
       Scene* sc = new Scene( m_device->deviceClass()->channels() );
       sc->setName(text);

       for(unsigned int n = 0; n < m_device->deviceClass()->channels(); n++)
	 {
	   // Get values from device / HJu
	   sc->set(n, m_device->getChannelValue(n));
	 }

       if (m_deviceSource == "DeviceClass")
	 {
	   // If deviceclass is selected, save to device class
	   m_device->deviceClass()->addFunction(sc);
	 }
       else
	 {
	   // otherwise save to device / HJu
	   m_device->addFunction(sc);
	 }

       m_availableScenesComboBox->insertItem(text);
       m_availableScenesComboBox->setCurrentItem(m_availableScenesComboBox->count()-1);
       slotSceneActivated(0);
     }
   else
     {
       ASSERT(false);
     }
}




void SceneEditor::slotSaveClicked()
{
  ASSERT(m_currentScene != NULL);

  // Take values from device because it returns real values for
  // sure and they are unsigned chars and it is much simpler this way / HJu
  for (unsigned short i = 0; i < m_device->deviceClass()->channels(); i++)
    {
      m_currentScene->set(i, m_device->getChannelValue(i));
    }

  if (m_deviceSource == "DeviceClass")
    {
      m_device->deviceClass()->saveToFile();
    }

  m_setStatusText("saved", QColor( 0, 0, 255));
}


void SceneEditor::slotDeviceRadio_clicked()
{
  Function* f;

  QList <Function> fl = m_device->functions();
  m_availableScenesComboBox->clear();

  for (f = fl.first(); f != NULL; f = fl.next())
    {
      if (f->type() == Function::Scene)
	{
	  m_availableScenesComboBox->insertItem(f->name());
	}
    }
  m_deviceSource = "Device";
}


void SceneEditor::slotClassRadio_clicked()
{
  Function* f;
  QList <Function> fl = m_device->deviceClass()->functions();
  m_availableScenesComboBox->clear();

  for (f = fl.first(); f != NULL; f = fl.next())
    {
      if (f->type() == Function::Scene)
	{
	  m_availableScenesComboBox->insertItem(f->name());
	}
    }

  m_deviceSource = "DeviceClass";
}



void SceneEditor::m_selectFunctions(QList <Function> fl)
{
  Function* f;
  for (f = fl.first(); f != NULL; f = fl.next())
    {
      if( f->typeString() == "Scene")
	m_availableScenesComboBox->insertItem(f->name());
    }
  m_setStatusText( "unchanged",QColor( 0, 255, 100 ));
  slotSceneActivated(0);
}



void SceneEditor::m_setStatusText(QString text, QColor color)
{
  m_statusLabel->setPaletteForegroundColor( color );
  m_statusLabel->setText( text );
}
