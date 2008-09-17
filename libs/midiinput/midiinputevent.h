/*
  Q Light Controller
  midiinputevent.h
  
  Copyright (C) Heikki Junnila
  
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

#ifndef MIDIINPUTEVENT_H
#define MIDIINPUTEVENT_H

#include <QEvent>
#include "common/qlctypes.h"

class MIDIDevice;

class MIDIInputEvent : public QEvent
{
public:
	static const QEvent::Type eventType;

public:
	MIDIInputEvent(MIDIDevice* device, t_input input,
		       t_input_channel channel, t_input_value value,
		       bool alive);
	~MIDIInputEvent();

	MIDIDevice* m_device;
	t_input m_input;
	t_input_channel m_channel;
	t_input_value m_value;
	bool m_alive;
};

#endif
