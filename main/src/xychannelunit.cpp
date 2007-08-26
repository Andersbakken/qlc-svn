/*
  Q Light Controller
  xychannelunit.cpp
  
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

#include "xychannelunit.h"

#include "app.h"
#include "doc.h"
#include "fixture.h"

extern App* _app;

XYChannelUnit::XYChannelUnit()
{
}

XYChannelUnit::XYChannelUnit(const t_fixture_id fixtureID,
	const t_channel channel,
	const t_value lo,
	const t_value hi,
	const bool reverse) :

	m_fixtureID	( fixtureID ),
	m_channel	( channel  ),
	m_lo 		( lo       ),
	m_hi		( hi       ),
	m_reverse	( reverse  )
{
}
	
XYChannelUnit::~XYChannelUnit()
{
}
	
/**
 * Set the low limit for the channel
 *
 * @param lo The low limit
 */
void XYChannelUnit::setLo(t_value lo) { m_lo = lo; }

/**
 * Get the low limit
 */
t_value XYChannelUnit::lo() const { return m_lo; }

/**
 * Set the high limit for the channel
 *
 * @param hi The high limit
 */
void XYChannelUnit::setHi(t_value hi) { m_hi = hi; }

/**
 * Get the high limit
 */
t_value XYChannelUnit::hi() const { return m_hi; }
	
/**
 * Set the fixture id
 *
 * @param fixtureID The fixture id
 */
void XYChannelUnit::setFixtureID(t_fixture_id fixtureID) { m_fixtureID = fixtureID; }

/**
 * Get the fixture ID
 */
t_fixture_id XYChannelUnit::fixtureID() const { return m_fixtureID; }

/**
 * Convenience function to get the fixture associated
 * with the fixture ID
 */
Fixture* XYChannelUnit::fixture() const { return _app->doc()->fixture(m_fixtureID); }
	
/**
 * Set the relative channel number
 *
 * @param ch The channel
 */
void XYChannelUnit::setChannel(t_channel ch) { m_channel = ch; }

/**
 * Get the relative channel number
 */
t_channel XYChannelUnit::channel() const { return m_channel; }

/**
 * Set the channel value reverse status
 *
 * @param reverse true for reverse, false for normal
 */
void XYChannelUnit::setReverse(bool reverse) { m_reverse = reverse; }

/**
 * Get reverse status
 */
bool XYChannelUnit::reverse() const { return m_reverse; }
