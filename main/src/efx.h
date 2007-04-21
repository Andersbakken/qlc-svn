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

#include "common/types.h"
#include "function.h"
#include "scene.h"

class QPointArray;
class QDomDocument;

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
	 * Set a pointer to a point array for updating the
	 * changes when editing the function.
	 *
	 * @note Call this function with NULL after editing is finished!
	 *
	 * @param array The array to save the preview points to
	 */
	void setPreviewPointArray(QPointArray* array);

	/**
	 * Get the supported algorithms as a string list
	 *
	 * @note This is a static function
	 *
	 * @param algorithms A QStringList that shall contain the algorithms
	 */
	static void algorithmList(QStringList& list);

	/**
	 * Get the current algorithm
	 *
	 */
	QString algorithm();

	/**
	 * Set the algorithm
	 *
	 */
	void setAlgorithm(QString algorithm);

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
	 * Set the pattern rotation
	 *
	 * @param rot Pattern rotation (0-359)
	 */
	void setRotation(int rot);

	/**
	 * Get the pattern rotation
	 *
	 * @return Pattern rotation (0-359)
	 */
	int rotation();

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
	 * Set the lissajous pattern frequency  on the X-axis
	 *
	 * @param freq Pattern frequency (0-255)
	 */
	void setXFrequency(int freq);

	/**
	 * Get the lissajous pattern frequency on the X-axis
	 *
	 * @return Pattern offset (0-255)
	 */
	int xFrequency();

	/**
	 * Set the lissajous pattern frequency  on the Y-axis
	 *
	 * @param freq Pattern frequency (0-255)
	 */
	void setYFrequency(int freq);

	/**
	 * Get the lissajous pattern frequency on the Y-axis
	 *
	 * @return Pattern offset (0-255)
	 */
	int yFrequency();

	/**
	 * Returns true when lissajous has been selected
	 */
	bool isFrequencyEnabled();

	/**
	 * Set the lissajous pattern phase on the X-axis
	 *
	 * @param phase Pattern phase (0-255)
	 */
	void setXPhase(int phase);

	/**
	 * Get the lissajous pattern phase on the X-axis
	 *
	 * @return Pattern phase (0-255)
	 */
	int xPhase();

	/**
	 * Set the lissajous pattern phase on the Y-axis
	 *
	 * @param phase Pattern phase (0-255)
	 */
	void setYPhase(int phase);

	/**
	 * Get the lissajous pattern phase on the Y-axis
	 *
	 * @return Pattern phase (0-255)
	 */
	int yPhase();

	/**
	 * Returns true when lissajous has been selected
	 */
	bool isPhaseEnabled();

	/**
	 * Set a channel from a device to be used as the X axis.
	 *
	 * @param channel Relative number of the channel used as the X axis
	 */
	void setXChannel(t_channel channel);

	/**
	 * Get the channel used as the X axis.
	 *
	 */
	t_channel xChannel();

	/**
	 * Set a channel from a device to be used as the Y axis.
	 *
	 * @param channel Relative number of the channel used as the Y axis
	 */
	void setYChannel(t_channel channel);

	/**
	 * Get the channel used as the Y axis.
	 *
	 */
	t_channel yChannel();

	/**
	 * Set the run order
	 *
	 * @param runOrder Run Order
	 */
	void setRunOrder(RunOrder runOrder);

	/**
	 * Get the run order
	 *
	 */
	RunOrder runOrder();

	/**
	 * Set the running direction
	 *
	 * @param dir Direction
	 */
	void setDirection(Direction dir);

	/**
	 * Get the direction
	 *
	 */
	Direction direction();

	/**
	 * Set the modulation speed bus
	 *
	 */
	void setModulationBus(t_bus_id bus);

	/**
	 * Get the modulation speed bus
	 *
	 */
	t_bus_id modulationBus();
  
	/**
	 * Set the start scene if Enabled
	 *
	 */
	void setStartScene(t_function_id id);
  
	/**
	 * Get the id for start scene
	 *
	 */
	t_function_id startScene();
  
	/**
	 * Start scene enabled
	 *
	 */
	void setStartSceneEnabled(bool set);

	/**
	 * Get start scene enabled status
	 *
	 */
	bool startSceneEnabled();

	/**
	 * Set the stop scene if Enabled
	 *
	 */
	void setStopScene(t_function_id id);
  
	/**
	 * Get the id for stop scene
	 *
	 */
	t_function_id stopScene();
  
	/**
	 * Stop scene enabled
	 *
	 */
	void setStopSceneEnabled(bool set);

	/**
	 * Get stop scene enabled status
	 *
	 */
	bool stopSceneEnabled();

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
	 * Save the function's contents to an XML document
	 *
	 * @param doc The QDomDocument to save to
	 */
	void saveXML(QDomDocument* doc);

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
	 * Updates the preview points (if necessary)
	 *
	 */
	void updatePreview();

	/**
	 * Function pointer for the point calculation function.
	 * This pointer is replaced by the appropriate function pointer
	 * depending on the chosen algorithm.
	 */
	void (*pointFunc) (EFX* efx, float iterator, float* x, float* y);

	/**
	 * Calculate a single point in a circle based on
	 * the value of iterator (which is basically a step number)
	 *
	 * @note This is a static function
	 *
	 * @param efx The EFX function using this
	 * @param iterator Step number
	 * @param x Holds the calculated X coordinate
	 * @param y Holds the calculated Y coordinate
	 */
	static void circlePoint(EFX* efx, float iterator, float* x, float* y);

	/**
	 * Calculate a single point in a eight pattern based on
	 * the value of iterator (which is basically a step number)
	 *
	 * @note This is a static function
	 *
	 * @param efx The EFX function using this
	 * @param iterator Step number
	 * @param x Holds the calculated X coordinate
	 * @param y Holds the calculated Y coordinate
	 */
	static void eightPoint(EFX* efx, float iterator, float* x, float* y);

	/**
	 * Calculate a single point in a line pattern based on
	 * the value of iterator (which is basically a step number)
	 *
	 * @note This is a static function
	 *
	 * @param efx The EFX function using this
	 * @param iterator Step number
	 * @param x Holds the calculated X coordinate
	 * @param y Holds the calculated Y coordinate
	 */
	static void linePoint(EFX* efx, float iterator, float* x, float* y);

	/**
	 * Calculate a single point in a triangle pattern based on
	 * the value of iterator (which is basically a step number)
	 *
	 * @note This is a static function
	 *
	 * @param efx The EFX function using this
	 * @param iterator Step number
	 * @param x Holds the calculated X coordinate
	 * @param y Holds the calculated Y coordinate
	 */
	static void trianglePoint(EFX* efx, float iterator, float* x, float* y);

	/**
	 * Calculate a single point in a diamond pattern based on
	 * the value of iterator (which is basically a step number)
	 *
	 * @note This is a static function
	 *
	 * @param efx The EFX function using this
	 * @param iterator Step number
	 * @param x Holds the calculated X coordinate
	 * @param y Holds the calculated Y coordinate
	 */
	static void diamondPoint(EFX* efx, float iterator, float* x, float* y);

	/**
	 * Calculate a single point in a lissajous pattern based on
	 * the value of iterator (which is basically a step number)
	 *
	 * @note This is a static function
	 *
	 * @param efx The EFX function using this
	 * @param iterator Step number
	 * @param x Holds the calculated X coordinate
	 * @param y Holds the calculated Y coordinate
	 */
	static void lissajousPoint(EFX* efx, float iterator, float* x, float* y);

	/**
	 * Write the actual calculated coordinate data to
	 * event buffer.
	 */
	void setPoint(t_value x, t_value y);

	/**
	 *Rotate a point of the pattern by rot degrees
	 *Do scaling of height and width
	 */
	void rotateAndScale(EFX* efx, float *x, float *y, int rot);

 protected:
	/**
	 * Pattern width, see \ref setWidth
	 */
	float m_width;

	/**
	 * Pattern height, see \ref setHeight
	 */
	float m_height;

	/**
	 * Pattern X offset, see \ref setXOffset
	 */
	float m_xOffset;

	/**
	 * Pattern Y offset, see \ref setXOffset
	 */
	float m_yOffset;
  
	/**
	 * Pattern rotation, see \ref setRotation
	 */
	int m_rotation;
  
	/**
	 * Lissajous pattern X frequency, see \ref setXFrequency
	 */
	float m_xFrequency;

	/**
	 * Lissajous pattern Y frequency, see \ref setYFrequency
	 */
	float m_yFrequency;

	/**
	 * Lissajous pattern X phase, see \ref setXPhase
	 */
	float m_xPhase;

	/**
	 * Lissajous pattern Y phase, see \ref setYPhase
	 */
	float m_yPhase;

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

	/**
	 * Run order (or mode): loop, ping-pong or single-shot
	 *
	 */
	RunOrder m_runOrder;

	/**
	 * Direction: forwards or backwards
	 *
	 */
	Direction m_direction;

	/**
	 * The modulation speed bus
	 *
	 */
	t_bus_id m_modulationBus;

	/**
	 * Start (initialisation) scene
	 *
	 */
	t_function_id m_startSceneID;
	bool m_startSceneEnabled;

	/**
	 * Stop (de-initialisation) scene
	 *
	 */
	t_function_id m_stopSceneID;
	bool m_stopSceneEnabled;
    
	/**
	 * The point array which holds a preview of the pattern
	 * generated by this EFX function when run. This should be
	 * used only from the EFX Editor and set to NULL after finished
	 * editing.
	 */
	QPointArray* m_previewPointArray;

	/**
	 * Algorithm used by the current EFX function
	 */
	QString m_algorithm;

	/**
	 * The size of one step derived from \ref m_cycleDuration. If m_cycleDuration
	 * is 64, then this is 1/64.
	 */
	float m_stepSize;

	/**
	 * How long (how many timer ticks) the function should take to complete
	 * one cycle. One timer tick is 1/KFrequency (== 1/64th of a second). So
	 * setting KFrequency as the cycle duration means that the function takes
	 * one second to complete a full cycle.
	 */
	float m_cycleDuration;

	/**
	 * Channel data that is written to the event buffer
	 */
	t_buffer_data* m_channelData;

	/**
	 * Run-time device address. Don't use for anything else!
	 */
	t_channel m_address;
};

#endif
