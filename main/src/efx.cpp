/*
  Q Light Controller
  efx.cpp
  
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

#include <qfile.h>
#include <qptrlist.h>
#include <time.h>
#include <qapplication.h>
#include <qpointarray.h>
#include <assert.h>
#include <math.h>

#include "app.h"
#include "doc.h"
#include "bus.h"
#include "device.h"
#include "deviceclass.h"
#include "eventbuffer.h"
#include "efx.h"
#include "functionconsumer.h"
#include "../../libs/common/outputplugin.h"

extern App* _app;

/* Supported EFX algorithms */
static const char* KCircleAlgorithmName    ( "Circle"    );
static const char* KEightAlgorithmName     ( "Eight"     );
static const char* KLineAlgorithmName      ( "Line"      );
static const char* KDiamondAlgorithmName   ( "Diamond"   );
static const char* KTriangleAlgorithmName  ( "Triangle"  );
static const char* KLissajousAlgorithmName ( "Lissajous" );

/**
 * Standard constructor
 */
EFX::EFX() :
  Function            ( Function::EFX ),

  pointFunc           ( NULL ),
  
  m_width             ( 127 ),
  m_height            ( 127 ),
  m_xOffset           ( 127 ),
  m_yOffset           ( 127 ),
  m_rotation	      ( 0 ),

  m_xFrequency        ( 2 ),
  m_yFrequency        ( 3 ),
  m_xPhase            ( 1.5707963267 ),
  m_yPhase            ( 0 ),

  m_customParameters     ( NULL ),
  m_customParameterCount ( 0 ),

  m_xChannel          ( KChannelInvalid ),
  m_yChannel          ( KChannelInvalid ),

  m_runOrder          ( EFX::Loop ),
  m_direction         ( EFX::Forward ),

  m_modulationBus     ( KBusIDDefaultHold ),

  m_previewPointArray ( NULL ),

  m_algorithm         ( KCircleAlgorithmName ),

  m_stepSize          ( 0 ),
  m_cycleDuration     ( KFrequency ),

  m_channelData       ( NULL ),

  m_address           ( KChannelInvalid )
{
  /* Set Default Fade as the speed bus */
  setBus(KBusIDDefaultFade);
}

/**
 * Standard destructor
 */
EFX::~EFX()
{
  stop();

  m_startMutex.lock();
  while (m_running)
    {
      m_startMutex.unlock();
      sched_yield();
      m_startMutex.lock();
    }
  m_startMutex.unlock();
}

/**
 * Set a pointer to a point array for updating the
 * changes when editing the function.
 *
 * @note Call this function with NULL after editing is finished!
 *
 * @param array The array to save the preview points to
 */
void EFX::setPreviewPointArray(QPointArray* array)
{
  m_previewPointArray = array;
}

/**
 * Updates the preview points (if necessary)
 *
 */
void EFX::updatePreview()
{
  if (m_previewPointArray == NULL)
    {
      return;
    }

  int stepCount = 128;
  int step = 0;
  float stepSize = (float)(1) / ((float)(stepCount) / (M_PI * 2.0));

  float i = 0;
  float *x = new float;
  float *y = new float;

  /* Resize the array to contain stepCount points */
  m_previewPointArray->resize(stepCount);

  if (m_algorithm == KCircleAlgorithmName)
    {
      /* Draw a preview of a circle */
      for (i = 0; i < (M_PI * 2.0); i += stepSize)
	{
	  circlePoint(this, i, x, y);
	  m_previewPointArray->setPoint(step++, 
					static_cast<int> (*x),
					static_cast<int> (*y));
	}
    }
  else if (m_algorithm == KEightAlgorithmName)
    {
      /* Draw a preview of a eight */
      for (i = 0; i < (M_PI * 2.0); i += stepSize)
	{
	  eightPoint(this, i, x, y);
	  m_previewPointArray->setPoint(step++, 
					static_cast<int> (*x),
					static_cast<int> (*y));
	}
    }
  else if (m_algorithm == KLineAlgorithmName)
    {
      /* Draw a preview of a line */
      for (i = 0; i < (M_PI * 2.0); i += stepSize)
	{
	  linePoint(this, i, x, y);
	  m_previewPointArray->setPoint(step++, 
					static_cast<int> (*x),
					static_cast<int> (*y));
	}
    }
  else if (m_algorithm == KDiamondAlgorithmName)
    {
      /* Draw a preview of a diamond */
      for (i = 0; i < (M_PI * 2.0); i += stepSize)
	{
	  diamondPoint(this, i, x, y);
	  m_previewPointArray->setPoint(step++, 
					static_cast<int> (*x),
					static_cast<int> (*y));
	}
    }
  else if (m_algorithm == KTriangleAlgorithmName)
    {
      /* Draw a preview of a triangle */
      for (i = 0; i < (M_PI * 2.0); i += stepSize)
	{
	  trianglePoint(this, i, x, y);
	  m_previewPointArray->setPoint(step++, 
					static_cast<int> (*x),
					static_cast<int> (*y));
	}
    }
  else if (m_algorithm == KLissajousAlgorithmName)
    {
      /* Draw a preview of a lissajous */
      for (i = 0; i < (M_PI * 2.0); i += stepSize)
	{
	  lissajousPoint(this, i, x, y);
	  m_previewPointArray->setPoint(step++, 
					static_cast<int> (*x),
					static_cast<int> (*y));
	}
    }
  else
    {
      m_previewPointArray->resize(0);
    }

  delete x;
  delete y;
}


/**
 * Get the supported algorithms as a string list
 *
 * @note This is a static function
 *
 * @param algorithms A QStrList that shall contain the algorithms
 */
void EFX::algorithmList(QStringList& list)
{
  list.clear();
  list.append(KCircleAlgorithmName);
  list.append(KEightAlgorithmName);
  list.append(KLineAlgorithmName);
  list.append(KDiamondAlgorithmName);
  /* list.append(KTriangleAlgorithmName); */
  list.append(KLissajousAlgorithmName);
}

/** 
 * Get the current algorithm
 *
 * @return Name of the current algorithm. See @ref algorithmList
 */
QString EFX::algorithm()
{
  return m_algorithm;
}

/**
 * Set the current algorithm
 *
 * @param algorithm One of the strings returned by @ref algorithmList
 */
void EFX::setAlgorithm(QString algorithm)
{
  QStringList list;

  EFX::algorithmList(list);

  list = list.grep(algorithm);
  if (list.isEmpty())
    {
      qDebug("Invalid algorithm for EFX: " + algorithm);
      m_algorithm = KCircleAlgorithmName;
    }
  else
    {
      m_algorithm = QString(algorithm);
    }

  updatePreview();
}

/**
 * Set the pattern width
 *
 * @param width Pattern width (0-255)
 */
void EFX::setWidth(int width)
{
  m_width = static_cast<double> (width);
  updatePreview();
}

/**
 * Get the pattern width
 *
 * @return Pattern width (0-255)
 */
int EFX::width()
{
  return static_cast<int> (m_width);
}

/**
 * Set the pattern height
 *
 * @param height Pattern height (0-255)
 */
void EFX::setHeight(int height)
{
  m_height = static_cast<double> (height);
  updatePreview();
}

/**
 * Get the pattern height
 *
 * @return Pattern height (0-255)
 */
int EFX::height()
{
  return static_cast<int> (m_height);
}

  /**
   * Set the pattern rotation
   *
   * @param rot Pattern rotation (0-359)
   */
 void EFX::setRotation(int rot)
{
  m_rotation = static_cast<int> (rot);
  updatePreview();
}

/**
 * Get the pattern rotation
 *
 * @return Pattern rotation (0-359)
 */
int EFX::rotation()
{
  return static_cast<int> (m_rotation);
}


/**
 * Set the pattern offset on the X-axis
 *
 * @param offset Pattern offset (0-255; 127 is middle)
 */
void EFX::setXOffset(int offset)
{
  m_xOffset = static_cast<double> (offset);
  updatePreview();
}

/**
 * Get the pattern offset on the X-axis
 *
 * @return Pattern offset (0-255; 127 is middle)
 */
int EFX::xOffset()
{
  return static_cast<int> (m_xOffset);
}

/**
 * Set the pattern offset on the Y-axis
 *
 * @param offset Pattern offset (0-255; 127 is middle)
 */
void EFX::setYOffset(int offset)
{
  m_yOffset = static_cast<double> (offset);
  updatePreview();
}

/**
 * Get the pattern offset on the Y-axis
 *
 * @return Pattern offset (0-255; 127 is middle)
 */
int EFX::yOffset()
{
  return static_cast<int> (m_yOffset);
}

/**
 * Set the lissajous pattern frequency  on the X-axis
 *
 * @param freq Pattern frequency (0-255)
 */
void EFX::setXFrequency(int freq)
{
  m_xFrequency = freq;
  updatePreview();
}

/**
 * Get the lissajous pattern frequency on the X-axis
 *
 * @return Pattern offset (0-255)
 */
int EFX::xFrequency()
{
  return static_cast<int> (m_xFrequency);
}

/**
 * Set the lissajous pattern frequency  on the Y-axis
 *
 * @param freq Pattern frequency (0-255)
 */
void EFX::setYFrequency(int freq)
{
  m_yFrequency = freq;
  updatePreview();
}

/**
 * Get the lissajous pattern frequency on the Y-axis
 *
 * @return Pattern offset (0-255)
 */
int EFX::yFrequency()
{
  return static_cast<int> (m_yFrequency);
}

/**
 * Set the lissajous pattern phase on the X-axis
 *
 * @param phase Pattern phase (0-255)
 */
void EFX::setXPhase(int phase)
{
  m_xPhase = static_cast<float> (phase * M_PI / 180.0);
  updatePreview();
}

/**
 * Get the lissajous pattern phase on the X-axis
 *
 * @return Pattern phase (0-255)
 */
int EFX::xPhase()
{
  return static_cast<int> (m_xPhase * 180.0 / M_PI);
}

/**
 * Set the lissajous pattern phase on the Y-axis
 *
 * @param phase Pattern phase (0-255)
 */
void EFX::setYPhase(int phase)
{
  m_yPhase = static_cast<float> (phase * M_PI) / 180.0;
  updatePreview();
}

/**
 * Get the lissajous pattern phase on the Y-axis
 *
 * @return Pattern phase (0-255)
 */
int EFX::yPhase()
{
  return static_cast<int> (m_yPhase * 180.0 / M_PI);
}

/**
 * Returns true when lissajous has been selected
 */
bool EFX::isFrequencyEnabled()
{
  if (m_algorithm == KLissajousAlgorithmName)
    {
      return true;
    }
  else
    {
      return false;
    }
}

/**
 * Returns true when lissajous has been selected
 */
bool EFX::isPhaseEnabled()
{
  if (m_algorithm == KLissajousAlgorithmName)
    {
      return true;
    }
  else
    {
      return false;
    }
}

/**
 * A list of integers to set as custom pattern
 * parameters (i.e. such parameters that are not
 * common to all patterns)
 *
 * @param params Array of integer values
 * @param len Array length
 */
void EFX::setCustomParameters(int* params, int len)
{
  /* Delete existing parameter array */
  if (m_customParameters)
    {
      delete [] m_customParameters;
      m_customParameters = NULL;
      m_customParameterCount = 0;
    }

  /* Create new array if necessary */
  if (len > 0)
    {
      m_customParameters = new int[len];
      memcpy(m_customParameters, params, len);
      m_customParameterCount = len;
    }

  updatePreview();
}

/**
 * Get the array of custom parameters
 *
 * @param len The length of the array.
 * @return The internal parameter array. Do not modify.
 */
const int* EFX::customParameters(int* len)
{
  if (len)
    {
      *len = m_customParameterCount;
      return static_cast <const int*> (m_customParameters);
    }
  else
    {
      assert(false);
    }
}

/**
 * Set a channel from a device to be used as the X axis.
 *
 * @param channel Relative number of the channel used as the X axis
 */
void EFX::setXChannel(t_channel channel)
{
  Device* device = _app->doc()->device(m_deviceID);
  assert(device);

  if (channel < (t_channel) device->deviceClass()->channels()->count())
    {
      m_xChannel = channel;
      updatePreview();
    }
  else
    {
      qDebug("Invalid channel number!");
      assert(false);
    }
}

/**
 * Set a channel from a device to be used as the Y axis.
 *
 * @param channel Relative number of the channel used as the Y axis
 */
void EFX::setYChannel(t_channel channel)
{
  Device* device = _app->doc()->device(m_deviceID);
  assert(device);

  if (channel < (t_channel) device->deviceClass()->channels()->count())
    {
      m_yChannel = channel;
      updatePreview();
    }
  else
    {
      qDebug("Invalid channel number!");
      assert(false);
    }
}

/**
 * Get the channel used as the X axis.
 *
 */
t_channel EFX::xChannel()
{
  return m_xChannel;
}

/**
 * Get the channel used as the Y axis.
 *
 */
t_channel EFX::yChannel()
{
  return m_yChannel;
}

/**
 * Set the run order
 *
 * @param runOrder Run Order
 */
void EFX::setRunOrder(EFX::RunOrder runOrder)
{
  m_runOrder = runOrder;
}

/**
 * Get the run order
 *
 */
EFX::RunOrder EFX::runOrder()
{
  return m_runOrder;
}

/**
 * Set the running direction
 *
 * @param dir Direction
 */
void EFX::setDirection(EFX::Direction dir)
{
  m_direction = dir;
}

/**
 * Get the direction
 *
 */
EFX::Direction EFX::direction()
{
  return m_direction;
}

/**
 * Set the modulation speed bus
 *
 */
void EFX::setModulationBus(t_bus_id bus)
{
  if (bus > KBusIDMin && bus < KBusCount)
    {
      m_modulationBus = bus;
    }
}

/**
 * Get the modulation speed bus
 *
 */
t_bus_id EFX::modulationBus()
{
  return m_modulationBus;
}
  
/**
 * Copy function contents from another function
 *
 * @param efx EFX function from which to copy contents to this function
 * @param toDevice The new parent for this function
 */
bool EFX::copyFrom(EFX* efx, t_device_id toDevice)
{
  assert(efx);

  if (toDevice == KNoID || toDevice == m_deviceID)
    {
      // Same device
      /* TODO */
    }
  else
    {
      // Different device
      /* TODO */
    }

  return false;
}

/**
 * Called by Doc when saving the workspace file. Saves this function's
 * contents to the given file.
 *
 * @param file File to save to
 */
void EFX::saveToFile(QFile &file)
{
  QString s;
  QString t;

  // Comment line
  s = QString("# Function entry\n");
  file.writeBlock((const char*) s, s.length());

  // Entry type
  s = QString("Entry = Function") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Name
  s = QString("Name = ") + name() + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Type
  s = QString("Type = ") + Function::typeToString(m_type) + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // ID
  t.setNum(m_id);
  s = QString("ID = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Bus ID
  t.setNum(m_busID);
  s = QString("Bus = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Device ID
  t.setNum(m_deviceID);
  s = QString("Device = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Algorithm
  s = QString("Algorithm = ") + algorithm() + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Width
  t.setNum(width());
  s = QString("Width = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Height
  t.setNum(height());
  s = QString("Height = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Rotation
  t.setNum(rotation());
  s = QString("Rotation = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // X Offset
  t.setNum(xOffset());
  s = QString("XOffset = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Y Offset
  t.setNum(yOffset());
  s = QString("YOffset = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // X Frequency
  t.setNum(xFrequency());
  s = QString("XFrequency = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Y Frequency
  t.setNum(yFrequency());
  s = QString("YFrequency = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // X Phase
  t.setNum(xPhase());
  s = QString("XPhase = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Y Phase
  t.setNum(yPhase());
  s = QString("YPhase = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // X Channel
  t.setNum(xChannel());
  s = QString("XChannel = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Y Channel
  t.setNum(yChannel());
  s = QString("YChannel = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Run Order
  t.setNum((int) runOrder());
  s = QString("RunOrder = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Direction
  t.setNum((int) direction());
  s = QString("Direction = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Modulation Bus
  t.setNum((int) modulationBus());
  s = QString("ModulationBus = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());
}

/**
 * Parse function contents from a list of string tokens. This is
 * called by Doc when loading a workspace file.
 *
 * @param list List of string tokens (item,value,item,value,item...)
 */
void EFX::createContents(QPtrList <QString> &list)
{
  QString t;
  
  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
        {
          s = list.prev();
          break;
        }
      else if (*s == QString("Algorithm"))
	{
	  setAlgorithm(*(list.next()));
	}
      else if (*s == QString("Width"))
	{
	  setWidth(list.next()->toInt());
	}
      else if (*s == QString("Height"))
	{
	  setHeight(list.next()->toInt());
	}
     else if (*s == QString("Rotation"))
	{
	  setRotation(list.next()->toInt());
	}
      else if (*s == QString("XOffset"))
	{
	  setXOffset(list.next()->toInt());
	}
      else if (*s == QString("YOffset"))
	{
	  setYOffset(list.next()->toInt());
	}
      else if (*s == QString("XFrequency"))
	{
	  setXFrequency(list.next()->toInt());
	}
      else if (*s == QString("YFrequency"))
	{
	  setYFrequency(list.next()->toInt());
	}
      else if (*s == QString("XPhase"))
	{
	  setXPhase(list.next()->toInt());
	}
      else if (*s == QString("YPhase"))
	{
	  setYPhase(list.next()->toInt());
	}
      else if (*s == QString("XChannel"))
	{
	  setXChannel(list.next()->toInt());
	}
      else if (*s == QString("YChannel"))
	{
	  setYChannel(list.next()->toInt());
	}
      else if (*s == QString("RunOrder"))
	{
	  setRunOrder((RunOrder) list.next()->toInt());
	}
      else if (*s == QString("Direction"))
	{
	  setDirection((Direction) list.next()->toInt());
	}
      else if (*s == QString("ModulationBus"))
	{
	  setModulationBus((t_bus_id) list.next()->toInt());
	}
      else
        {
          // Unknown keyword, skip
          list.next();
        }
    }
}

/**
 * This is called by buses for each function when the
 * bus value is changed.
 * 
 * @param id ID of the bus that has changed its value
 * @param value Bus' new value
 */
void EFX::busValueChanged(t_bus_id id, t_bus_value value)
{
  if (id != m_busID)
    {
      /* Not our bus */
      return;
    }

  m_startMutex.lock();

  /* Basically number of steps required to complete a full cycle */
  m_cycleDuration = static_cast<double> (value);

  /* Size of one step */
  m_stepSize = (double)(1) / ((double)(m_cycleDuration) / (M_PI * 2));

  m_startMutex.unlock();
}

/**
 * Prepare this function for running. This is called when
 * the user sets the mode to Operate. Basically allocates everything
 * that is needed to run the function.
 */
void EFX::arm()
{
  /* Allocate space for channel data set to eventbuffer.
   * There are only two channels to set.
   */
  if (m_channelData == NULL)
    m_channelData = new t_value[2 * 2];

  /* Allocate space for the event buffer.
   * There are only two channels to set.
   */
  if (m_eventBuffer == NULL)
    m_eventBuffer = new EventBuffer(2 * sizeof(t_value) * 2,
				    KFrequency >> 1, /* == KFrequency / 2 */
				    sizeof(t_value));

  /* Set the run time address for channel data */
  if (_app->doc()->device(m_deviceID))
    {
      m_address = _app->doc()->device(m_deviceID)->address();
    }
  else
    {
      qDebug("No device for EFX: " + Function::name());
    }

  m_stopped = false;

  /* Choose a point calculation function depending on the algorithm */
  if (m_algorithm == KCircleAlgorithmName)
    {
      pointFunc = circlePoint;
    }
  else if (m_algorithm == KEightAlgorithmName)
    {
      pointFunc = eightPoint;
    }
  else if (m_algorithm == KLineAlgorithmName)
    {
      pointFunc = linePoint;
    }
  else if (m_algorithm == KTriangleAlgorithmName)
    {
      pointFunc = trianglePoint;
    }
  else if (m_algorithm == KDiamondAlgorithmName)
    {
      pointFunc = diamondPoint;
    }
  else if (m_algorithm == KLissajousAlgorithmName)
    {
      pointFunc = lissajousPoint;
    }
  else
    {
      /* There's something wrong, don't run this function */
      pointFunc = NULL;
      m_stopped = true;

      qDebug("Unknown algorithm used in EFX: " + m_name);
    }
}

/**
 * Free all run-time allocations. This is called respectively when
 * the user sets the mode back to Design.
 */
void EFX::disarm()
{
  if (m_channelData) delete [] m_channelData;
  m_channelData = NULL;

  if (m_eventBuffer) delete m_eventBuffer;
  m_eventBuffer = NULL;

  m_address = KChannelInvalid;

  pointFunc = NULL;
}

/**
 * Called by FunctionConsumer after the function has stopped running.
 * Usually notifies parent function and/or virtual console that the
 * function has been stopped.
 */
void EFX::cleanup()
{
  m_stopped = false;
  
  if (m_virtualController)
    {
      QApplication::postEvent(m_virtualController,
			      new FunctionStopEvent(m_id));
      m_virtualController = NULL;
    }
  
  if (m_parentFunction)
    {
      m_parentFunction->childFinished();
      m_parentFunction = NULL;
    }
  
  m_startMutex.lock();
  m_running = false;
  m_startMutex.unlock();
}

/**
 * Pre-run initialization that is run just before the function is started.
 */
void EFX::init()
{
  t_bus_value speed;

  m_removeAfterEmpty = false;

  // Get speed
  Bus::value(m_busID, speed);
  busValueChanged(m_busID, speed);
  
  // Append this function to running functions' list
  _app->functionConsumer()->cue(this);
}

/**
 * The worker thread that takes care of filling the function's
 * buffer with event data
 */
void EFX::run()
{
  float i = 0;
  float* x = new float;
  float* y = new float;

  // Initialize this function for running
  init();

  while (!m_stopped)
    {
      for (i = 0; i < (M_PI * 2.0) && !m_stopped; i += m_stepSize)
	{
	  /* Calculate the next point */
	  pointFunc(this, i, x, y);

	  /* Write the point to event buffer */
	  setPoint(static_cast<t_value> (*x), static_cast<t_value> (*y));
	}
    }

  /* These are no longer needed */
  delete x;
  delete y;

  if (m_stopped)
    {
      /* The function was stopped elsewhere, purge the contents of
       * the eventbuffer so that the function is removed from queue
       * as soon as possible */
      m_eventBuffer->purge();
    }
  
  /* Finished */
  m_removeAfterEmpty = true;
}

/**
 * Calculate a single point in a circle pattern based on
 * the value of iterator (which is basically a step number)
 *
 * @note This is a static function
 *
 * @param efx The EFX function using this
 * @param iterator Step number
 * @param x Holds the calculated X coordinate
 * @param y Holds the calculated Y coordinate
 */
void EFX::circlePoint(EFX* efx, float iterator, float* x, float* y)
{
  *x = cos(iterator + M_PI_2);
  *y = cos(iterator);
  efx->rotateAndScale(efx, x, y, efx->m_rotation);
}

/**
 * Calculate a single point in an eight pattern based on
 * the value of iterator (which is basically a step number)
 *
 * @note This is a static function
 *
 * @param efx The EFX function using this
 * @param iterator Step number
 * @param x Holds the calculated X coordinate
 * @param y Holds the calculated Y coordinate
 */
void EFX::eightPoint(EFX* efx, float iterator, float* x, float* y)
{
  *x = cos((iterator * 2) + M_PI_2);
  *y = cos(iterator);
  efx->rotateAndScale(efx, x, y, efx->m_rotation);
}

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
void EFX::linePoint(EFX* efx, float iterator, float* x, float* y)
{
  /* TODO: It's a simple line, I don't think we need cos() :) */
  *x = cos(iterator);
  *y = cos(iterator);
  efx->rotateAndScale(efx, x, y, efx->m_rotation);
}

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
void EFX::trianglePoint(EFX* efx, float iterator, float* x, float* y)
{
  /* TODO !!! */
  *x = cos(iterator);
  *y = sin(iterator);
  efx->rotateAndScale(efx, x, y, efx->m_rotation);
}

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
void EFX::diamondPoint(EFX* efx, float iterator, float* x, float* y)
{
  *x = pow(cos(iterator - M_PI_2), 3);
  *y = pow(cos(iterator), 3);
  efx->rotateAndScale(efx, x, y, efx->m_rotation);
}

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
void EFX::lissajousPoint(EFX* efx, float iterator, float* x, float* y)
{
  *x = cos((efx->m_xFrequency * iterator) - efx->m_xPhase);
  *y = cos((efx->m_yFrequency * iterator) - efx->m_yPhase);
  efx->rotateAndScale(efx, x, y, efx->m_rotation);
}

/**
 * Write the actual calculated coordinate data to
 * event buffer.
 */
void EFX::setPoint(t_value x, t_value y)
{
  m_channelData[0] = m_address + m_xChannel;
  m_channelData[1] = x;

  m_channelData[2] = m_address + m_yChannel;
  m_channelData[3] = y;

  m_eventBuffer->put(m_channelData);
}


/**
 * Rotate a single point in a  pattern by
 * the value of rot, scale height and width
 *
 *
 * @param x Holds the calculated X coordinate
 * @param y Holds the calculated Y coordinate
 * @param rot Amount of rotation in degrees
 */
void EFX::rotateAndScale(EFX* efx, float* x, float* y, int rot)
{
  float xx, yy;
  xx = *x;
  yy = *y;
  float r = M_PI/180 * float (rot);
  *x = efx->m_xOffset + (xx * cos(r) + yy  * sin(r)) * efx->m_width ;
  *y = efx->m_yOffset + (-xx * sin(r) + yy * cos(r))  * efx->m_height;
}


