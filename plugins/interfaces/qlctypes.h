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

/** Win32 needs these in order to be able to link to dynamic libraries */
#ifdef QLC_EXPORT
#  define QLC_DECLSPEC Q_DECL_EXPORT
#else
#  define QLC_DECLSPEC Q_DECL_IMPORT
#endif

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
 * Number of supported universes
 */
const quint32 KUniverseCount ( 4 );

/*****************************************************************************
 * Output lines
 *****************************************************************************/

/**
 * Invalid output line number
 */
const quint32 KOutputInvalid ( UINT_MAX );

/*****************************************************************************
 * Input universes
 *****************************************************************************/

/**
 * Number of input universes
 */
const quint32 KInputUniverseCount ( 4 );

/*****************************************************************************
 * Input lines
 *****************************************************************************/

/**
 * Invalid input line number
 */
const quint32 KInputInvalid ( UINT_MAX );

/*****************************************************************************
 * Input channels
 *****************************************************************************/

/**
 * Smallest input channel
 */
const quint32 KInputChannelMin ( 0 );

/**
 * Largest input channel
 */
const quint32 KInputChannelMax ( UINT_MAX - 1);

/**
 * Largest input channel
 */
const quint32 KInputChannelInvalid ( UINT_MAX );

#endif
