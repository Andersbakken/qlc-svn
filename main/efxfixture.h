/*
  Q Light Controller
  efxfixture.h
  
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

#ifndef EFXFIXTURE_H
#define EFXFIXTURE_H

#include "common/qlctypes.h"
#include "function.h"

class EFX;
class Scene;

class EFXFixture
{
	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	EFXFixture(EFX* parent, t_fixture_id fxi_id, int index, int order,
		   Function::Direction direction, Scene* startScene,
		   Scene* stopScene);
	~EFXFixture();

	/** Reset the fixture when the EFX is stopped */
	void reset();

	/** Check, whether this EFXFixture is ready (no more events) */
	bool isReady() const { return m_ready; }

protected:
	/** The EFX function that this fixture belongs to */
	EFX* m_parent;

	/** The ID of the fixture that is EFXFixture represents */
	t_fixture_id m_fixture;
	
	/** This fixture's starting index in m_channelData */
	int m_index;

	/** This fixture's order in serial propagation mode */
	int m_order;

	/** This fixture's current direction */
	Function::Direction m_direction;

	/** The scene that is used to initialize the fixtures involved */
	Scene* m_startScene;

	/** The scene that is used to de-initialize the fixtures involved */
	Scene* m_stopScene;

	/** When the start scene is run, the EFXFixture has been initialized */
	bool m_initialized;

	/** When running in single shot mode, the fixture is marked ready
	    after it has completed a full cycle. */
	bool m_ready;

	/*********************************************************************
	 * Channels
	 *********************************************************************/
public:
	void setLsbPanChannel(t_channel ch);
	void setMsbPanChannel(t_channel ch);

	void setLsbTiltChannel(t_channel ch);
	void setMsbTiltChannel(t_channel ch);

	void updateSkipThreshold();

	bool isValid();

protected:
	/**
	 * This fixture's current position in the pattern (a point on a
	 * circle's circumference)
	 */
	float m_iterator;

	/**
	 * This iterator is incremented until it is >= m_skipThreshold.
	 * After that, m_iterator is incremented. Used for serial propagation.
	 */
	float m_skipIterator;

	/**
	 * This is basically the index of a point in the EFX's pattern,
	 * where this fixture will start doing its stuff. Used for serial
	 * propagation.
	 */
	float m_skipThreshold;

	/**
	 * The current pan value
	 */
	float m_panValue;

	/**
	 * The current tilt value
	 */
	float m_tiltValue;

	/**
	 * (Index + 0)th channel in m_channelData
	 */
	t_channel m_lsbPanChannel;

	/**
	 * (index + 1)th channel in m_channelData
	 */
	t_channel m_msbPanChannel;

	/**
	 * (index + 2)th channel in m_channelData
	 */
	t_channel m_lsbTiltChannel;

	/**
	 * (index + 3)th channel in m_channelData
	 */
	t_channel m_msbTiltChannel;

	/*********************************************************************
	 * Running
	 *********************************************************************/
public:
	/**
	 * Calculate the next step for this fixture and put the values to $data
	 */
	void nextStep(t_buffer_data* data);

protected:
	/**
	 * Write this EFXFixture's channel data to event buffer
	 */
	void setPoint(t_buffer_data* data);
};

#endif
