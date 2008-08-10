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
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

XYChannelUnit::XYChannelUnit()
{
	m_fixtureID = KNoID;
	m_channel = 0;
	m_lo = 0;
	m_hi = 255;
	m_reverse = false;
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
	
void XYChannelUnit::setLo(t_value lo)
{
	m_lo = lo;
}

t_value XYChannelUnit::lo() const
{
	return m_lo;
}

void XYChannelUnit::setHi(t_value hi)
{
	m_hi = hi;
}

t_value XYChannelUnit::hi() const
{
	return m_hi;
}
	
void XYChannelUnit::setFixtureID(t_fixture_id fixtureID)
{
	m_fixtureID = fixtureID;
}

t_fixture_id XYChannelUnit::fixtureID() const
{
	return m_fixtureID;
}

Fixture* XYChannelUnit::fixture() const
{
	return _app->doc()->fixture(m_fixtureID);
}
	
void XYChannelUnit::setChannel(t_channel ch)
{
	m_channel = ch;
}

t_channel XYChannelUnit::channel() const
{
	return m_channel;
}

void XYChannelUnit::setReverse(bool reverse)
{
	m_reverse = reverse;
}

bool XYChannelUnit::reverse() const
{
	return m_reverse;
}
