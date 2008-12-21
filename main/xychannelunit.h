/*
  Q Light Controller
  xychannelunit.h

  Copyright (c) Stefan Krumm, Heikki Junnila

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

#include "common/qlctypes.h"

class Fixture;

class XYChannelUnit
{
public:
	XYChannelUnit();
	XYChannelUnit(const t_fixture_id fixtureID,
		      const t_channel channel,
		      const t_value lo,
		      const t_value hi,
		      const bool reverse);
	XYChannelUnit(const XYChannelUnit& unit);
	~XYChannelUnit();

	/** Assignment operator */
	XYChannelUnit& operator=(const XYChannelUnit& unit);

	enum
	{
		FileElementFixture = 0,
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
	 * Set the fixture id
	 *
	 * @param fixtureID The fixture id
	 */
	void setFixtureID(t_fixture_id fixtureID);

	/**
	 * Get the fixture ID
	 */
	t_fixture_id fixtureID() const;

	/**
	 * Convenience function to get the fixture associated
	 * with the fixture ID
	 */
	Fixture* fixture() const;

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
	t_fixture_id m_fixtureID;
	t_channel m_channel;

	t_value m_lo;
	t_value m_hi;

	bool m_reverse;
};

#endif
