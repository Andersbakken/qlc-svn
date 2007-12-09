/*
  Q Light Controller
  outputplugin.h

  Copyright (c) Heikki Junnila

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

#ifndef INPUTPLUGIN_H
#define INPUTPLUGIN_H

#include <qevent.h>

#include "common/plugin.h"
#include "common/types.h"

const int KInputEvent        ( QEvent::User + 10 );

class InputEvent : public QCustomEvent
{ 
public:
        InputEvent(int id, int channel, int value) 
	: QCustomEvent( KInputEvent )
	{ m_id = id; m_channel=channel; m_value=value; }


	//virtual ~InputEvent();
	int m_id;
	int m_channel;
	int m_value;
	int id() const {return m_id;}
	int channel() const {return m_channel;}
	int value() const {return m_value;}
};



class InputPlugin : public Plugin
{
	Q_OBJECT

public:
	InputPlugin(int id);
	virtual ~InputPlugin();
	virtual void feedBack(int id, int channel, int value){};

};

#endif
