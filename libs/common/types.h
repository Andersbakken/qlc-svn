/*
  Q Light Controller
  types.h

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

#ifndef TYPES_H
#define TYPES_H

#include <limits.h>

/*****************************************************************************
 * Generic
 *****************************************************************************/

/**
 * Generic invalid ID
 */
const int KNoID ( -1 );

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

/**
 * Fixture ID type
 */
typedef int t_fixture_id;

/**
 * Maximum number of fixtures
 */
const t_fixture_id KFixtureArraySize ( 128 );

/*****************************************************************************
 * Function
 *****************************************************************************/

/**
 * Function ID type
 */
typedef int t_function_id;

/**
 * Maximum number of functions
 */
const t_function_id KFunctionArraySize ( 4096 );

/*****************************************************************************
 * DMX channel
 *****************************************************************************/

/**
 * Type for channel numbers
 */
typedef unsigned short t_channel;

/**
 * Number of supported universes
 */
const t_channel KUniverseCount ( 4 );

/**
 * Smallest channel number
 */
const t_channel KChannelMin ( 0 );

/**
 * Total number of channels (in all universes)
 */
const t_channel KChannelMax ( 512 * KUniverseCount );

/**
 * Maximum number of channels for a single fixture
 */
const t_channel KFixtureChannelsMax ( 64 );

/**
 * Invalid channel number (must be larger than KChannelMax!)
 */
const t_channel KChannelInvalid ( USHRT_MAX );

/**
 * QLCChannel's control byte for 16bit pan/tilt etc. (i.e. MSB/LSB)
 * 0 = the first 8 bits, 1 = bits 9-16 ... 255 = bits 2033-2040 (yikes!)
 */
typedef unsigned char t_controlbyte;

/**
 * Low control byte for 16bit DMX values
 */
const t_controlbyte KControl16LSB ( 0 );

/**
 * High control byte for 16bit DMX values
 */
const t_controlbyte KControl16MSB ( 1 );

/*****************************************************************************
 * DMX value
 *****************************************************************************/

/**
 * Channel value type
 */
typedef unsigned char t_value;

/**
 * Smallest channel value
 */
const t_value KChannelValueMin ( 0 );

/**
 * Largest channel value
 */
const t_value KChannelValueMax ( 255 );

/*****************************************************************************
 * Event buffer
 *****************************************************************************/

/**
 * Event buffer data type
 */
typedef unsigned int t_buffer_data;

/*****************************************************************************
 * Bus
 *****************************************************************************/

/**
 * Bus ID type
 */
typedef short t_bus_id;

/**
 * Bus value type
 */
typedef unsigned long t_bus_value;

/**
 * Smallest bus ID
 */
const t_bus_id KBusIDMin         ( 0 );

/**
 * Number of buses
 */
const t_bus_id KBusCount         ( 32 );

/**
 * Invalid bus ID
 */
const t_bus_id KBusIDInvalid     ( -1 );

/**
 * The default fade bus ID
 */
const t_bus_id KBusIDDefaultFade ( KBusIDMin );

/**
 * The default hold bus ID
 */
const t_bus_id KBusIDDefaultHold ( KBusIDMin + 1 );

/*****************************************************************************
 * Axes
 *****************************************************************************/

/**
 * Axis type
 */
typedef unsigned char t_axis;

/**
 * X Axis
 */
const t_axis KAxisX ( 0 );

/**
 * Y Axis
 */
const t_axis KAxisY ( 1 );

/**
 * Z Axis (you never know...)
 */
const t_axis KAxisZ ( 2 );

/*****************************************************************************
 * Function consumer engine
 *****************************************************************************/

const int KFrequency ( 64 );

#endif
