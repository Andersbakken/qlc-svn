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
 * Device
 *****************************************************************************/

/**
 * Device ID type
 */
typedef int t_device_id;

/**
 * Maximum number of devices
 */
const t_device_id KDeviceArraySize ( 128 );

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
 * Largest number of universes (optimistic)
 */
const t_channel KUniverseCount ( 8 );

/**
 * Smallest channel number
 */
const t_channel KChannelMin ( 0 );

/**
 * Total number of channels (in all universes)
 */
const t_channel KChannelMax ( 512 * KUniverseCount );

/**
 * Invalid channel number (must be larger than KChannelMax!)
 */
const t_channel KChannelInvalid ( USHRT_MAX );

/*****************************************************************************
 * Event buffer
 *****************************************************************************/

/**
 * Event buffer data type
 */
typedef unsigned int t_buffer_data;

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
const t_bus_id KBusIDMin         (             0 );

/**
 * Number of buses
 */
const t_bus_id KBusCount         (            32 );

/**
 * Invalid bus ID
 */
const t_bus_id KBusIDInvalid     (            -1 );

/**
 * The default fade bus ID
 */
const t_bus_id KBusIDDefaultFade ( KBusIDMin     );

/**
 * The default hold bus ID
 */
const t_bus_id KBusIDDefaultHold ( KBusIDMin + 1 );

/*****************************************************************************
 * Fixture
 *****************************************************************************/

/**
 * LogicalChannel's control byte for 16bit pan/tilt etc. (i.e. MSB/LSB)
 * 0 = the first 8 bits, 1 = bits 9-16 ... 255 = bits 2033-2040 (yikes!)
 */
typedef unsigned char t_controlbyte;

/*****************************************************************************
 * Virtual console
 *****************************************************************************/

/**
 * Virtual Console widget ID type
 */
typedef unsigned long t_vc_id;

/**
 * Smallest virtual console widget ID number
 */
const t_vc_id KVCIDMin ( 1 );

/*****************************************************************************
 * Function consumer engine
 *****************************************************************************/

const int KFrequency ( 64 );

#endif
