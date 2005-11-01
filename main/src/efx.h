/*
  Q Light Controller
  efx.h
  
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

#ifndef EFX_H
#define EFX_H

#include "function.h"
#include "scene.h"
#include "types.h"

/**
 * An EFX (effects) function that is used to create
 * more complex automation especially for moving lights
 */
class EFX : public Function
{
 public:
  EFX();
  ~EFX();

  /**
   * List of different algorithm currently supported by
   * the EFX function.
   */
  enum Algorithm
    {
      Circle = 0,
      Eight,
      Line,
      Square,
      Triangle,
      Lissajous
    };

  /**
   * Set the pattern width
   *
   * @param width Pattern width (0-255)
   */
  void setWidth(int width);

  /**
   * Get the pattern width
   *
   * @return Pattern width (0-255)
   */
  int width();

  /**
   * Set the pattern height
   *
   * @param height Pattern height (0-255)
   */
  void setHeight(int height);

  /**
   * Get the pattern height
   *
   * @return Pattern height (0-255)
   */
  int height();

  /**
   * Set the pattern offset on the X-axis
   *
   * @param offset Pattern offset (0-255; 127 is middle)
   */
  void setXOffset(int offset);

  /**
   * Get the pattern offset on the X-axis
   *
   * @return Pattern offset (0-255; 127 is middle)
   */
  int xOffset();

  /**
   * Set the pattern offset on the Y-axis
   *
   * @param offset Pattern offset (0-255; 127 is middle)
   */
  void setYOffset(int offset);

  /**
   * Get the pattern offset on the Y-axis
   *
   * @return Pattern offset (0-255; 127 is middle)
   */
  int yOffset();

  /**
   * An array of integers to set as custom pattern parameters
   * (i.e. such parameters that are not common to all algorithms)
   *
   * @param params Array of integer values
   * @param len Array length
   */
  void setCustomParameters(int* params, int len);

  /**
   * Get the array of custom parameters
   *
   * @param len The length of the array.
   * @return The internal parameter array. Do not modify.
   */
  const int* customParameters(int* len);

  /**
   * Set a channel from a device to be used as the X axis.
   *
   * @param channel Relative number of the channel used as the X axis
   */
  void setXChannel(t_channel channel);

  /**
   * Set a channel from a device to be used as the Y axis.
   *
   * @param channel Relative number of the channel used as the Y axis
   */
  void setYChannel(t_channel channel);

  /**
   * Copy function contents from another function
   *
   * @param efx EFX function from which to copy contents to this function
   * @param toDevice The new parent for this function
   */
  bool copyFrom(EFX* efx, t_device_id toDevice = KNoID);

  /**
   * Called by Doc when saving the workspace file. Saves this function's
   * contents to the given file.
   *
   * @param file File to save to
   */
  void saveToFile(QFile &file);

  /**
   * Parse function contents from a list of string tokens. This is
   * called by Doc when loading a workspace file.
   *
   * @param list List of string tokens (item,value,item,value,item...)
   */
  void createContents(QPtrList <QString> &list);

  /**
   * This is called by buses for each function when the
   * bus value is changed.
   * 
   * @param id ID of the bus that has changed its value
   * @param value Bus' new value
   */
  void busValueChanged(t_bus_id id, t_bus_value value);

  /**
   * Prepare this function for running. This is called when
   * the user sets the mode to Operate. Basically allocates everything
   * that is needed to run the function.
   */
  void arm();

  /**
   * Free all run-time allocations. This is called respectively when
   * the user sets the mode back to Design.
   */
  void disarm();

  /**
   * Called by FunctionConsumer after the function has stopped running.
   * Usually notifies parent function and/or virtual console that the
   * function has been stopped.
   */
  void cleanup();

 protected:
  /**
   * Pre-run initialization that is run just before the function is started.
   */
  void init();
  
  /**
   * The worker thread that takes care of filling the function's
   * buffer with event data
   */
  void run();

  /**
   * Worker thread for creating circle patterns
   */
  void circle();

  /**
   * Worker thread for creating eight patterns
   */
  void eight();

  /**
   * Worker thread for creating line patterns
   */
  void line();

  /**
   * Worker thread for creating square patterns
   */
  void square();

  /**
   * Worker thread for creating triangle patterns
   */
  void triangle();

  /**
   * Worker thread for creating lissajous patterns
   */
  void lissajous();

  /**
   * Write the actual calculated coordinate data to
   * event buffer.
   */
  void setPoint(int x, int y);

 protected:
  /**
   * Pattern width, see \ref setWidth
   */
  double m_width;

  /**
   * Pattern height, see \ref setHeight
   */
  double m_height;

  /**
   * Pattern X offset, see \ref setXOffset
   */
  double m_xOffset;

  /**
   * Pattern Y offset, see \ref setXOffset
   */
  double m_yOffset;

  /**
   * The size of one step derived from \ref m_cycleDuration. If m_cycleDuration
   * is 64, then this is 1/64.
   */
  double m_stepSize;

  /**
   * How long (how many timer ticks) the function should take to complete
   * one cycle. One timer tick is 1/KFrequency (== 1/64th of a second). So
   * setting KFrequency as the cycle duration means that the function takes
   * one second to complete a full cycle.
   */
  double m_cycleDuration;

  /**
   * Parameters that are not common to all algorithms.
   */
  int* m_customParameters;

  /**
   * Size of the custom parameter array
   */
  int m_customParameterCount;

  /**
   * Channel used for X coordinate data
   */
  t_channel m_xChannel;

  /**
   * Channel used for Y coordinate data
   */
  t_channel m_yChannel;

 protected:
  /**
   * Algorithm used by the current EFX function
   */
  EFX::Algorithm m_algorithm;

  /**
   * Channel data that is written to the event buffer
   */
  t_value* m_channelData;
};

#endif
