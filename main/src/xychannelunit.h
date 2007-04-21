/*
  Q Light Controller
  xychannelunit.h
  
  Copyright (C) 2005 Heikki Junnila, Stefan Krumm
  
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

#ifndef XYCHANNELUNIT_H
#define XYCHANNELUNIT_H

#include "common/types.h"

class Device;

class XYChannelUnit
{
public:
	XYChannelUnit();
	XYChannelUnit(const t_device_id deviceID,
		const t_channel channel,
		const t_value lo,
		const t_value hi,
		const bool reverse);
	
	~XYChannelUnit();
	
	enum {
		FileElementDevice = 0,
		FileElementChannel,
		FileElementLo,
		FileElementHi,
		FileElementReverse
	} ElementOrder;
		
	/**
	 * Set the low limit for the channel
	 *
	 * @param lo The low limit
	 */
	void setLo(t_value lo);

	/**
	 * Get the low limit
	 */
	t_value lo() const;
	
	/**
	 * Set the high limit for the channel
	 *
	 * @param hi The high limit
	 */
	void setHi(t_value hi);

	/**
	 * Get the high limit
	 */
	t_value hi() const;
	
	/**
	 * Set the device id
	 *
	 * @param deviceID The device id
	 */
	void setDeviceID(t_device_id deviceID);

	/**
	 * Get the device ID
	 */
	t_device_id deviceID() const;

	/**
	 * Convenience function to get the device associated
	 * with the device ID
	 */
	Device* device() const;
	
	/**
	 * Set the relative channel number
	 *
	 * @param ch The channel
	 */
	void setChannel(t_channel ch);

	/**
	 * Get the relative channel number
	 */
	t_channel channel() const;

	/**
	 * Set the channel value reverse status
	 *
	 * @param reverse true for reverse, false for normal
	 */
	void setReverse(bool reverse);

	/**
	 * Get reverse status
	 */
	bool reverse() const;

protected:
	t_device_id m_deviceID;
	t_channel m_channel;	
	
	t_value m_lo;
	t_value m_hi;
	
	bool m_reverse;
};

#endif
