/*
  Q Light Controller
  uDMXout.cpp
  
  Copyright (c)	2008, Lutz Hillebrand (ilLUTZminator)
    
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

#ifdef WIN32
#include <windows.h>
#else
#include <errno.h>
#endif

#include <QApplication>
#include <QMessageBox>
#include <QStringList>
#include <QPalette>
#include <QDebug>
#include <QString>
#include <QColor>
#include <QSettings>

#define CONF_FILE "./uDMXout.conf"
#define LOG_FILE "./uDMXout.log"

#include "configureudmxout.h"
#include "udmxout.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void uDMXOut::init()
{
  m_hLog = new QFile(LOG_FILE) ;
  m_hLog->open(QIODevice::WriteOnly) ;

  //Debug(DEBUG_ALL, "%s %d %s()", __FILE__, __LINE__, __FUNCTION__) ;  
  
  for(int n = 0; n < MAX_UDMX_DEVICES; n++)
    m_device[n] = NULL ;

  m_deviceName    = QString("uDMX");
  m_firstUniverse = 0;
  m_numOfDevices  = 0;
  m_configDir     = QString("./");

  m_Channels      = 256 ;       // number of channels
  m_Debug         = 0 ;       // NO debug to console
  m_LastInvalidDevice = -1 ;

  // initialize values
  memset(m_values, 0, KChannelMax) ; 
  
  loadSettings();
  
   // initialize USB
  usb_set_debug(1);
  usb_init(); 
}

int uDMXOut::usbStringGet(usb_dev_handle *dev, int index, 
                               int langid, char *buf, int buflen)
{
  char    buffer[256];
  int     rval, i;

  if((rval = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR, 
                             (USB_DT_STRING << 8) + index, langid, 
                             buffer, sizeof(buffer), 1000)) < 0)
    return rval;

  if(buffer[1] != USB_DT_STRING)
    return 0;

  if((unsigned char)buffer[0] < rval)
    rval = (unsigned char)buffer[0];

  rval /= 2;
  /* lossy conversion to ISO Latin1 */
  for(i=1;i<rval;i++)
  {
    if(i > buflen)  /* destination buffer overflow */
      break;
    buf[i-1] = buffer[2 * i];
    if(buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
      buf[i-1] = '?';
  }
  buf[i-1] = 0;
  return i-1;
}

void uDMXOut::rescanDevices()
{
  //Debug(DEBUG_ALL, "%s %d %s()", __FILE__, __LINE__, __FUNCTION__) ;  
}

/*****************************************************************************
 * Plugin open/close
 *****************************************************************************/

void uDMXOut::open(t_output output)
{
  //Debug(DEBUG_ALL, "%s %d %s()", __FILE__, __LINE__, __FUNCTION__) ;  
  
  QString txt ;
  int     n = m_firstUniverse ;
  
   struct usb_bus      *bus;
  struct usb_device   *dev;
  usb_dev_handle      *handle = 0;
  
  m_numOfDevices = 0 ;
   
  usb_find_busses();
  usb_find_devices();

  /* Search for uDMX device on all busses */
  for(bus = usb_busses; bus; bus = bus->next)
  {
    for(dev = bus->devices; dev; dev = dev->next)
    {
      if((dev->descriptor.idVendor  == USBDEV_SHARED_VENDOR) && 
         (dev->descriptor.idProduct == USBDEV_SHARED_PRODUCT))
      {
        char    string[256];
        int     len;

        //Debug(DEBUG_INFO, "%s %d %s -> Shared Vendor found", __FILE__, __LINE__, __FUNCTION__) ;

        /* we need to open the device in order to query strings */
        handle = usb_open(dev);
        if(!handle)
        {
          //Debug(DEBUG_ERROR, "%s %d %s -> ERROR: cannot open USB device: %s",
          //      __FILE__, __LINE__, __FUNCTION__, usb_strerror()) ;
          continue;
        }

        /* now find out whether the device is uDMX */
        len = usbStringGet(handle, dev->descriptor.iManufacturer, 0x0409, string, sizeof(string));
        if(len < 0)
        {
          //Debug(DEBUG_ERROR, "%s %d %s -> ERROR: cannot query manufacturer for device: %s",
          //      __FILE__, __LINE__, __FUNCTION__, usb_strerror()) ;
          goto skipDevice;
        }

        //Debug(DEBUG_INFO, "%s %d %s -> got Descriptor.iManu [%s]",
        //     __FILE__, __LINE__, __FUNCTION__, string) ;

        if(strcmp(string, "www.anyma.ch") != 0)
        {
          goto skipDevice;
        }

        len = usbStringGet(handle, dev->descriptor.iProduct, 0x0409, string, sizeof(string));
        if(len < 0)
        {
          //Debug(DEBUG_ERROR, "%s %d %s -> ERROR: cannot query product for device: %s",
          //      __FILE__, __LINE__, __FUNCTION__, usb_strerror()) ;
          goto skipDevice;
        }

        //Debug(DEBUG_INFO, "%s %d %s -> got Descriptor.iProd [%s]",
        //      __FILE__, __LINE__, __FUNCTION__, string) ;

        if(strcmp(string, "uDMX") != 0)
        {
skipDevice:
          usb_close(handle) ;
          handle = NULL ;
        }
        else
        {
          // uDMX device found !
          // store handle in array
          m_device[n] = handle ;

          m_numOfDevices++ ;
          n++ ;

          if ((m_numOfDevices >= MAX_UDMX_DEVICES) ||
              (n >= (MAX_UDMX_DEVICES - 1) ))
          {
            goto LoopEnd ;
          }
        }
      }
    }
  }

LoopEnd:

  if(!m_numOfDevices)
  {
    //Debug(DEBUG_ERROR, "%s %d %s() ERROR: no valid uDMX devices could be opened ! uDMX output is NOT available !", 
    //      __FILE__, __LINE__, __FUNCTION__);
  }
  else 
  {
    for(n = m_firstUniverse; n < (m_firstUniverse + m_numOfDevices); n++)
    {
      //Debug(DEBUG_INFO, "%s %d %s() uDMX available for universe %d", __FILE__, __LINE__, __FUNCTION__, n) ;

      writeRange(0, n * 512, m_values, m_Channels);
    }
  }

  //Debug(DEBUG_INFO, "%s %d %s() FirstUniverse %d, NumDevs %d", 
  //      __FILE__, __LINE__, __FUNCTION__, m_firstUniverse, m_numOfDevices ) ;

  return ;
}

void uDMXOut::close(t_output output)
{
  //Debug(DEBUG_ALL, "%s %d %s()", __FILE__, __LINE__, __FUNCTION__) ;  
}

bool uDMXOut::isOpen()
{
  //return (m_device[0] != NULL) ;
  return true ;
}

void uDMXOut::activate()
{
  //emit activated(this);
}

QStringList uDMXOut::outputs()
{
  //Debug(DEBUG_ALL, "%s %d %s()", __FILE__, __LINE__, __FUNCTION__) ;  
  
	QStringList list;

	for (int i = 0; i < 1 /*KUniverseCount*/; i++)
		list << QString("%1: uDMXout %1").arg(i + 1);

	return list;  
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString uDMXOut::name()
{
	return QString("uDMX Output");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void uDMXOut::configure()
{
  //Debug(DEBUG_ALL, "%s %d %s()", __FILE__, __LINE__, __FUNCTION__) ;  
  
	Configure_uDMXOut conf(NULL, this);
	//conf.exec();
  
  if (conf.exec() == QDialog::Accepted)
  {
    m_deviceName    = conf.device();

    // not supported yet
    //m_firstUniverse = conf->firstUniverse();
    m_firstUniverse = 0 ;

    m_Channels      = conf.Channels() ;
    m_Debug         = conf.Debug() ;

    saveSettings() ;
  }  
}

int uDMXOut::saveSettings()
{
  //Debug(DEBUG_ALL, "%s %d %s", __FILE__, __LINE__, __FUNCTION__) ;

  //Debug(DEBUG_INFO, "%s %d %s: Saving config to %s", __FILE__, __LINE__, __FUNCTION__, CONF_FILE ) ;
  
  QSettings *settings = new QSettings(CONF_FILE, QSettings::IniFormat);
  
  settings->beginGroup("uDMX");
  settings->setValue("Debug", m_Debug);    
  settings->setValue("FirstUniverse", m_firstUniverse);  
  settings->setValue("Channels", m_Channels);   
   
  delete settings ;

  return errno;
}

int uDMXOut::loadSettings()
{
  //Debug(DEBUG_ALL, "%s %d %s", __FILE__, __LINE__, __FUNCTION__) ;

  QSettings *settings = new QSettings(CONF_FILE, QSettings::IniFormat);
  
  settings->beginGroup("uDMX");
  m_Debug         = settings->value("Debug", 0).toInt();
  m_firstUniverse = settings->value("FirstUniverse", 0).toInt() ;  
  m_Channels      = settings->value("Channels", 1).toInt() ;   
   
  delete settings ;
  
  return 0;
}

/*****************************************************************************
 * Plugin status
 *****************************************************************************/

QString uDMXOut::infoText(t_output output)
{
	QString str;

	str += QString("<HTML>");
	str += QString("<HEAD>");
	str += QString("<TITLE>%1</TITLE>").arg(name());
	str += QString("</HEAD>");
	str += QString("<BODY>");

	if (output == KOutputInvalid)
	{
		str += QString("<H3>%1</H3>").arg(name());
		str += QString("<P>");
		str += QString("This plugin provides DMX output support for ");
		str += QString("uDMX devices");
		str += QString("</P>");
	}
  /*
	else if (m_devices.contains(output) == true)
	{
		str += QString("<H3>%1</H3>").arg(outputs()[output]);
		str += QString("<P>");
		str += QString("Device is operating correctly.");
		str += QString("</P>");
	}
  */

	str += QString("</BODY>");
	str += QString("</HTML>");

	return str;
}

/*****************************************************************************
 * Value Read/Write
 *****************************************************************************/

void uDMXOut::writeChannel(t_output output, t_channel channel, t_value value)
{
  //Debug(DEBUG_ALL, "%s %d %s(%d, %d, %d)", 
  //      __FILE__, __LINE__, __FUNCTION__, output, channel, value) ; 
        
  // Make it simple an actually use only ONE function
  // no difference between writeChannel and writeRange here ...
  m_values[channel] = value ;
  
  writeRange(output, channel, &m_values[channel], 1) ;
  
  return  ;        
}

void uDMXOut::writeRange(t_output output, t_channel address, t_value* values,
			   t_channel num)
{
  //Debug(DEBUG_ALL, "%s %d %s(%d, %d)", 
  //      __FILE__, __LINE__, __FUNCTION__, output, address) ; 
        
  QString           txt ;
  t_value *         pValue ;
  int               iChannelsToWrite, iBlockSize ;

  // store memory for output
  pValue = m_values + address ;
  memcpy(pValue, values, num * sizeof(t_value));

  // which one is the first universe to write to?
  int device       = int(address / 512) ;

  //what is the first channel on that device?
  // eg. address is 512, it would be channel 0 on device 1
  int firstChannel = address % 512 ;


  //write the first chunk
  int numToWrite = MIN(512 - firstChannel, num);

  while(num > 0)
  {
    if(device >= m_numOfDevices)
    {
      // not enough devices ...
      if ((device > m_LastInvalidDevice) ||
          (m_Debug >= DEBUG_ALL))
      {
        m_LastInvalidDevice = device ;
        //Debug(DEBUG_WARN, "%s %d %s() ERROR: tried to write to non-existing device %d",
        //      __FILE__, __LINE__, __FUNCTION__, device) ;
      }
      return ;
    }
    else
    {
      // OK, ready to send
      int nBytes ;

      iChannelsToWrite = numToWrite ;

      while (iChannelsToWrite > 0)
      {
        // size bigger than 256 creates too much "broken pipe" errors
        // so limit size to 256
        iBlockSize  = MIN(iChannelsToWrite, 256) ;

        nBytes = usb_control_msg(m_device[device], USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                                cmd_SetChannelRange, iBlockSize, firstChannel,
                                (char *)values, iBlockSize, 50000);
        //Debug(DEBUG_INFO, "%s %d %s(): %i bytes returned", __FILE__, __LINE__, __FUNCTION__, nBytes) ;
        if(nBytes < 0)
        {
          //Debug(DEBUG_ERROR, "%s %d %s() USB-ERROR: %s",
          //      __FILE__, __LINE__, __FUNCTION__, usb_strerror() ) ;
        }

        iChannelsToWrite  = iChannelsToWrite - (iBlockSize) ;
        firstChannel      = firstChannel     + (iBlockSize) ;
        pValue            = pValue           + (iBlockSize) ;
      }

      firstChannel = 0;
      device ++;
      num -= numToWrite;
      values += numToWrite;
      if(num >= 512)
        numToWrite = 512;
      else
        numToWrite = num;
    }
  }

  return ;
}

void uDMXOut::readChannel(t_output output, t_channel channel, t_value* value)
{
  //Debug(DEBUG_ALL, "%s %d %s(%d, %d, %d)", 
  //      __FILE__, __LINE__, __FUNCTION__, output, channel, *value) ;  

  value = (t_value *) m_values[channel] ;

  return ;
}

void uDMXOut::readRange(t_output output, t_channel address, t_value* values,
			  t_channel num)
{
  //Debug(DEBUG_ALL, "%s %d %s(%d, %d)", 
  //      __FILE__, __LINE__, __FUNCTION__, output, address) ;  

  for (t_channel i = 0; i < num; i++)
  {
    values[i] = m_values[i];
  }
}

// debugging
#if 0
bool uDMXOut::Debug(int debuglevel, char * format, ...)
{
  // That size should be big enough
  // little security risk of buffer overrun but another way
  // is too complicated ...
  char string[4096] ;
  
  if (m_Debug < debuglevel)
  {
    return true ;
  }

  va_list argzeiger ;
  va_start(argzeiger, format) ;
  vsprintf(string, format, argzeiger) ;
  va_end(argzeiger) ;
  
  QTextStream out(m_hLog);
#ifdef WIN32  
  out << string << "\r\n";  
#else
  out << string << "\n";  
#endif
  
  return true ;
}
#endif

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(uDMXout, uDMXOut)
