/*
  Q Light Controller
  efxfixture.cpp

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

#include <QDebug>
#include <math.h>

#include "eventbuffer.h"
#include "efxfixture.h"
#include "function.h"
#include "scene.h"
#include "efx.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

EFXFixture::EFXFixture(EFX* parent, t_fixture_id fxi_id, int index, int order,
		       Function::Direction direction, Scene* startScene,
		       Scene* stopScene)
{
	Q_ASSERT(parent != NULL);
	Q_ASSERT(fxi_id != KNoID);

	m_parent = parent;
	m_fixture = fxi_id;
	m_index = index;
	m_order = order;
	m_direction = direction;
	m_startScene = startScene;
	m_stopScene = stopScene;
	m_initialized = false;
	m_ready = false;

	m_skipIterator = 0;
	m_skipThreshold = 0;
	m_iterator = 0;
	m_panValue = 0;
	m_tiltValue = 0;

	m_lsbPanChannel = KChannelInvalid;
	m_msbPanChannel = KChannelInvalid;
	m_lsbTiltChannel = KChannelInvalid;
	m_msbTiltChannel = KChannelInvalid;
}

EFXFixture::~EFXFixture()
{
}

void EFXFixture::reset()
{
	m_panValue = 0;
	m_tiltValue = 0;
	m_skipIterator = 0;
	m_iterator = 0;
	m_initialized = false;
	m_ready = false;

	updateSkipThreshold();
}

/*****************************************************************************
 * Channels
 *****************************************************************************/

void EFXFixture::setLsbPanChannel(t_channel ch)
{
	m_lsbPanChannel = ch;
}

void EFXFixture::setMsbPanChannel(t_channel ch)
{
	m_msbPanChannel = ch;
}

void EFXFixture::setLsbTiltChannel(t_channel ch)
{
	m_lsbTiltChannel = ch;
}

void EFXFixture::setMsbTiltChannel(t_channel ch)
{
	m_msbTiltChannel = ch;
}

void EFXFixture::updateSkipThreshold()
{
	/* One EFX "round" is always (pi * 2) long. Divide this "circumference"
	   into as many steps as there are fixtures in this EFX. If there are
	   four fixtures, these steps end up in 12 o'clock, 3 o'clock,
	   6 o'clock and 9 o'clock etc.. */
	m_skipThreshold = float(m_order) *
		(float(M_PI * 2.0) / float(m_parent->fixtureCount()));
}

bool EFXFixture::isValid()
{
	if (m_msbPanChannel != KChannelInvalid &&
	    m_msbTiltChannel != KChannelInvalid)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void EFXFixture::nextStep(t_buffer_data* data)
{
	if (m_ready == true)
		return;

	if (m_parent->propagationMode() == EFX::Serial &&
	    m_skipIterator < m_skipThreshold)
	{
		m_skipIterator += m_parent->m_stepSize;
	}
	else
	{
		m_iterator += m_parent->m_stepSize;

		/* This fixture is now running. Initialize it */
		if (m_initialized == false)
		{
			m_initialized = true;

			if (m_startScene != NULL)
				m_startScene->writeValues(m_fixture);
		}
	}

	if (m_iterator < (M_PI * 2.0))
	{
		if (m_direction == Function::Forward)
		{
			m_parent->pointFunc(m_parent, m_iterator,
					    &m_panValue, &m_tiltValue);
		}
		else
		{
			m_parent->pointFunc(m_parent, (M_PI * 2.0) - m_iterator,
					    &m_panValue, &m_tiltValue);
		}

		setPoint(data);
	}
	else
	{
		/* Reset iterator, since we've gone a full circle */
		m_iterator = 0;

		if (m_parent->m_runOrder == Function::PingPong)
		{
			/* Reverse direction */
			if (m_direction == Function::Forward)
				m_direction = Function::Backward;
			else
				m_direction = Function::Forward;
		}
		else if (m_parent->m_runOrder == Function::SingleShot)
		{
			/* De-initialize the fixture and mark this as ready */
			if (m_stopScene != NULL)
				m_stopScene->writeValues(m_fixture);

			m_ready = true;
		}
	}
}

void EFXFixture::setPoint(t_buffer_data* data)
{
	/* Write coarse point data to event buffer */
	data[m_index + 0]  = m_msbPanChannel << 8;
	data[m_index + 0] |= static_cast <t_value> (m_panValue);

	data[m_index + 1]  = m_msbTiltChannel << 8;
	data[m_index + 1] |= static_cast <t_value> (m_tiltValue);

	/* Write fine point data to event buffer if applicable */
	if (m_lsbPanChannel != KChannelInvalid)
	{
		data[m_index + 2]  = m_lsbPanChannel << 8;
		data[m_index + 2] |= static_cast <t_value> 
			((m_panValue - floor(m_panValue)) * 255.0);
	}

	if (m_lsbTiltChannel != KChannelInvalid)
	{
		data[m_index + 3]  = m_lsbTiltChannel << 8;
		data[m_index + 3] |= static_cast <t_value> 
			((m_tiltValue - floor(m_tiltValue)) * 255.0);
	}
}

