/*
  Q Light Controller
  qlctypes.h

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

#ifndef QLCTYPES_H
#define QLCTYPES_H

#include <limits.h>

/*****************************************************************************
 * Utils
 *****************************************************************************/

#ifdef CLAMP
#undef CLAMP
#endif
/**
 * Ensure that x is between the limits set by low and high.
 * If low is greater than high the result is undefined.
 *
 * This is copied from GLib sources
 *
 * @param x The value to clamp
 * @param low The minimum allowed value
 * @param high The maximum allowed value
 * @return The value of x clamped to the range between low and high
 */
#define CLAMP(x, low, high) \
	(((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#ifdef MAX
#undef MAX
#endif
/**
 * Return the bigger value of the two given values
 *
 * @param x The first value to compare
 * @param y The second value to compare
 * @return The bigger one of the given values
 */
#define MAX(x, y) ((x < y) ? y : x)

#ifdef MIN
#undef MIN
#endif
/**
 * Return the smaller value of the two given values
 *
 * @param x The first value to compare
 * @param y The second value to compare
 * @return The smaller one of the given values
 */
#define MIN(x, y) ((x < y) ? x : y)

#ifdef SCALE
#undef SCALE
#endif
/**
 * Scale a value within a source range to an equal value within the
 * destination range. I.e. 5 on a scale of 0 - 10 would be 10 on a scale
 * of 0 - 20.
 */
#define SCALE(x, src_min, src_max, dest_min, dest_max) \
	((x - src_min) * (dest_max / (src_max - src_min)))

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
const t_fixture_id KFixtureArraySize ( 256 );

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
 * Output universes & channels
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
const t_channel KFixtureChannelsMax ( 128 );

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
 * Output lines
 *****************************************************************************/

/**
 * Output line type
 */
typedef unsigned short t_output;

/**
 * Smallest output line number
 */
const t_output KOutputMin ( 0 );

/**
 * Largest output line number
 */
const t_output KOutputMax ( USHRT_MAX - 1 );

/**
 * Invalid output line number
 */
const t_output KOutputInvalid ( USHRT_MAX );

/*****************************************************************************
 * Output values
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
const t_value KChannelValueMax ( UCHAR_MAX );

/*****************************************************************************
 * Input universes
 *****************************************************************************/

/**
 * Input universe type
 */
typedef unsigned short t_input_universe;

/**
 * Number of input universes
 */
const t_input_universe KInputUniverseCount ( 4 );

/**
 * Invalid input universe
 */
const t_input_universe KInputUniverseInvalid ( USHRT_MAX );

/*****************************************************************************
 * Input lines
 *****************************************************************************/

/**
 * Input line type
 */
typedef unsigned short t_input;

/**
 * Smallest input line number
 */
const t_input KInputMin ( 0 );

/**
 * Largest input line number
 */
const t_input KInputMax ( USHRT_MAX - 1 );

/**
 * Invalid input line number
 */
const t_input KInputInvalid ( USHRT_MAX );

/*****************************************************************************
 * Input channels
 *****************************************************************************/

/**
 * Input channel type
 */
typedef unsigned short t_input_channel;

/**
 * Smallest input channel
 */
const t_input_channel KInputChannelMin ( 0 );

/**
 * Largest input channel
 */
const t_input_channel KInputChannelMax ( USHRT_MAX - 1);

/**
 * Largest input channel
 */
const t_input_channel KInputChannelInvalid ( USHRT_MAX );

/*****************************************************************************
 * Input values
 *****************************************************************************/

/**
 * Input channel value type
 */
typedef unsigned char t_input_value;

/**
 * Smallest input channel value
 */
const t_input_value KInputValueMin ( 0 );

/**
 * Largest input channel value
 */
const t_input_value KInputValueMax ( UCHAR_MAX );

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

const int KFrequency ( 50 );

#endif
