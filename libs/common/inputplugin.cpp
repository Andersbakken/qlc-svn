/*
  Q Light Controller
  inputplugin.cpp

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

#include "inputplugin.h"

InputPlugin::InputPlugin() : Plugin()
{
	m_type = Plugin::InputType;
}

InputPlugin::~InputPlugin()
{
}

t_input InputPlugin::inputs()
{
	return 1;
}

t_input_channel InputPlugin::channels(t_input input)
{
	return 1;
}

void InputPlugin::feedBack(t_input input, t_input_channel channel,
			   t_input_value value)
{
}
