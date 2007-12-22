/*
  Q Light Controller
  inputplugin.h

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

#include "common/plugin.h"
#include "common/types.h"

/*****************************************************************************
 * InputPlugin
 *****************************************************************************/

class InputPlugin : public Plugin
{
	Q_OBJECT

public:
	InputPlugin();
	virtual ~InputPlugin();

	/**
	 * Get the number of inputs provided by the plugin
	 *
	 * @todo Make pure virtual
	 *
	 * @return Number of inputs provided by the plugin
	 */
	virtual t_input inputs();

	/**
	 * Get the number of channels provided by a plugin input
	 *
	 * @todo Make pure virtual
	 *
	 * @param input An input line whose number of channels to get
	 * @return Number of input channels in the given input
	 */
	virtual t_input_channel channels(t_input input);

	/**
	 * Send a value back to an input line's channel. This method can be
	 * used for example to move motorized sliders with QLC sliders.
	 *
	 * @param input The input line to send feedback to
	 * @param channel A channel in the input line to send feedback to
	 * @param value An input value to send back to the input channel
	 */
	virtual void feedBack(t_input input, t_input_channel channel,
			      t_input_value value);
	
signals:
	/**
	 * Signal a value change in an input's channel. This is THE signal that
	 * sends input values to QLC components.
	 *
	 * @param input An input line whose channel's value has changed
	 * @param channel A channel whose value has changed
	 * @param value The changed value
	 */
	void valueChanged(t_input input,
			  t_input_channel channel,
			  t_input_value value);
};

#endif
