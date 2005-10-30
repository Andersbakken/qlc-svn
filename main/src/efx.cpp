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

//
// Standard constructor
//
EFX::EFX() :
  Function            ( Function::EFX ),
  
  m_width             ( 127 ),
  m_height            ( 127 ),
  m_xOffset           ( 127 ),
  m_yOffset           ( 127 ),
  m_stepSize          ( 0 ),
  m_cycleDuration     ( KFrequency ), /* 1 second */
  m_orientation       ( 0 ),
  m_omegax            ( 1 ),
  m_omegay            ( 1 ),
  m_phasex            ( .0174532 ),
  m_phasey            ( .0174532 ),

  m_algorithm         ( EFX::Circle ),
  m_channelData       ( NULL )
{
  /* Set Default Fade as the speed bus */
  setBus(KBusIDDefaultFade);
}

//
// Standard destructor
//
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

//
// Copy EFX contents
//
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

//
// Assign this EFX to a device (or vice versa, whatever feels
// familiar to you) and allocate value arrays the size of device's
// channels
//
bool EFX::setDevice(t_device_id id)
{
  Device* device = _app->doc()->device(id);
  assert(device);

  /* TODO */

  return false;
}

//
// Save this function's contents to given file
//
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

  /* TODO: EFX Specific */
}

//
// Create the function contents from file that has been read into list
//
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
      /* TODO: EFX Specific */
      else
        {
          // Unknown keyword, skip
          list.next();
        }
    }
}

//
// Bus value has changed
//
void EFX::busValueChanged(t_bus_id id, t_bus_value value)
{
  if (id != m_busID)
    {
      return;
    }

  m_startMutex.lock();

  m_cycleDuration = static_cast<double> (value);

  switch (m_algorithm)
    {
    case EFX::Circle:
      {
	m_stepSize = (double)(1) / ((double)(m_cycleDuration) / (M_PI * 2));
	qDebug("1 / (%f / %f) = %f", m_cycleDuration, (M_PI * 2), m_stepSize);
      }
      break;

    case EFX::Eight:
      break;

    case EFX::Line:
      break;

    case EFX::Square:
      break;

    case EFX::Triangle:
      break;
      
    case EFX::Lissajous:
      break;

    default:
      break;
    }

  m_startMutex.unlock();
}

//
// Allocate space for some run time stuff
//
void EFX::arm()
{
  if (m_channelData == NULL)
    m_channelData = new t_value[m_channels * 2];

  if (m_eventBuffer == NULL)
    m_eventBuffer = new EventBuffer(m_channels);
}

//
// Free any run-time data
//
void EFX::disarm()
{
  if (m_channelData) delete [] m_channelData;
  m_channelData = NULL;

  if (m_eventBuffer) delete m_eventBuffer;
  m_eventBuffer = NULL;
}


//
// Get starting values
//
void EFX::init()
{
  t_bus_value speed;

  m_removeAfterEmpty = false;

  m_stopped = false;

  // Get speed
  Bus::value(m_busID, speed);
  busValueChanged(m_busID, speed);
  
  // Append this function to running functions' list
  _app->functionConsumer()->cue(this);
}

//
// The main scene producer thread. Nothing should be allocated here
// (not even local variables) to keep the function as fast as possible.
//
void EFX::run()
{
  // Initialize this function for running
  init();
  
  switch (m_type)
    {
    case EFX::Circle:
      circle();
      break;

    case EFX::Eight:
      eight();
      break;

    case EFX::Line:
      line();
      break;

    case EFX::Square:
      square();
      break;

    case EFX::Triangle:
      triangle();
      break;
      
    case EFX::Lissajous:
      lissajous();
      break;

    default:
      break;
    }

  if (m_stopped)
    {
      m_eventBuffer->purge();
    }
  
  // Finished
  m_removeAfterEmpty = true;
}

//
// This fuction must be called ONLY from functionconsumer AFTER
// this function is REALLY stopped.
//
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

//
// Set the values calculated by EFX functions
//
void EFX::setPoint(int x, int y)
{
  /* TODO */
}

//
// Render a circle
//
void EFX::circle()
{
  double i = 0;
  int ch = 0;
  int x = 0;
  int y = 0;

  while (!m_stopped)
    {
      for (i = 0; i < (M_PI * 2); i += m_stepSize)
	{
	  x = static_cast<int> (m_xOffset + (cos(i + M_PI_2) * m_width));
	  y = static_cast<int> (m_yOffset + (cos(i) * m_height));
	  
	  setPoint(x, y);
	}
    }
}

//
// Render an eight
//
void EFX::eight()
{
  while (!m_stopped)
    {
      /* TODO */
    }
}

//
// Render a line
//
void EFX::line()
{
  while (!m_stopped)
    {
      /* TODO */
    }
}

//
// Render a square
//
void EFX::square()
{
  while (!m_stopped)
    {
      /* TODO */
    }
}

//
// Render a triangle
//
void EFX::triangle()
{
  while (!m_stopped)
    {
      /* TODO */
    }
}

//
// Render a complex lissajous pattern
//
void EFX::lissajous()
{
  while (!m_stopped)
    {
      /* TODO */
    }
}
