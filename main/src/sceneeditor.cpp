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
#include "dmxchannel.h"
#include "logicalchannel.h"
#include "channelui.h"

#include <qlistbox.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
using namespace std;


SceneEditor::SceneEditor(DMXDevice* device, QWidget* parent, const char* name )
             : UI_SceneEditor( parent, name)
{
  m_device = device;
  m_currentScene = NULL;

  selectFunctions();
}

SceneEditor::~SceneEditor()
{

}

void SceneEditor::slotSceneChanged()
{
  setStatusText( "modified", QColor( 255, 0, 0 ) );
}

void SceneEditor::slotSceneActivated( int nr )
{
  Function* f;

  QList <Function> *fl = m_device->functions();
  for (f = fl->first(); f != NULL; f = fl->next())
    {
      if( f->name() == m_sceneList->currentText())
	{
	  setScene((Scene*) f);
	  m_currentScene = (Scene*) f;
	}
    }
  
  setStatusText( "unchanged", QColor( 0, 255, 100 ));
}


void SceneEditor::setScene(Scene* scene)
{
   ChannelUI* unit;
   QList <ChannelUI> ul = ((DMXDevice*) m_device)->getChannelUnitList();
   int n = 0;

   ASSERT(scene != NULL);

   for (unit = ul.first(); unit != NULL; unit = ul.next(), n++)
     {
       SceneValue value = scene->channelValue(n);
       if (value.type == Set || value.type == Fade)
	 {
	   unit->slotAnimateValueChange(value.value);
	   unit->setStatusButton(DMXChannel::On);
	 }
       else
	 {
	   unit->setStatusButton(DMXChannel::Off);
	 }
     }
}

void  SceneEditor::slotHideClicked()
{
  for (unsigned i = parentWidget()->width(); i > ChannelUI::width() * m_device->deviceClass()->channels()->count(); i--)
    {
      parentWidget()->resize(i, height());
    }
}

void  SceneEditor::slotNewClicked()
{
   bool ok = FALSE;
   QString text = QInputDialog::getText(
					tr( "Scene editor" ),
					tr( "Enter scene name" ),
					QLineEdit::Normal, QString::null, &ok, this );

   if ( ok && !text.isEmpty() )
     {
       Scene* sc = new Scene();
       sc->setName(text);
       sc->setDevice(m_device);
       for (unsigned int n = 0; n < m_device->deviceClass()->channels()->count(); n++)
	 {
	   // Get values from device / HJu
	   if (m_device->dmxChannel(n)->status() == DMXChannel::On)
	     {
	       sc->set(n, m_device->dmxChannel(n)->value(), Set);
	     }
	   else
	     {
	       sc->clear(n);
	     }
	 }

       // Save to device / HJu
       m_device->addFunction(sc);

       m_sceneList->insertItem(text);
       m_sceneList->setCurrentItem(m_sceneList->count()-1);
       slotSceneActivated(0);
     }
   else
     {
       qDebug("WTF?!");
     }
}




void SceneEditor::slotSaveClicked()
{
  ASSERT(m_currentScene != NULL);
  
  QList <ChannelUI> ul = ((DMXDevice*) m_device)->getChannelUnitList();

  // Take values from device because it returns real values for
  // sure and they are unsigned chars and it is much simpler this way / HJu
  for (unsigned short i = 0; i < m_device->deviceClass()->channels()->count(); i++)
    {
      if (m_device->dmxChannel(i)->status() == DMXChannel::On)
	{
	  m_currentScene->set(i, m_device->dmxChannel(i)->value(), Set);
	}
      else
	{
	  m_currentScene->clear(i);
	}
    }

  setStatusText("saved", QColor( 0, 0, 255));
}


void SceneEditor::selectFunctions()
{
  QList <Function> *fl = m_device->functions();

  for (Function* f = fl->first(); f != NULL; f = fl->next())
    {
      if (f->type() == Function::Scene)
	{
	  m_sceneList->insertItem(f->name());
	}
    }

  setStatusText( "unchanged",QColor( 0, 255, 100 ));
  slotSceneActivated(0);
}



void SceneEditor::setStatusText(QString text, QColor color)
{
  m_statusLabel->setPaletteForegroundColor( color );
  m_statusLabel->setText( text );
}
