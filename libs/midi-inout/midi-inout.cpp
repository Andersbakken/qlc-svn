/*
  Q Light Controller
  midi-inout.cpp
  
  Copyright (C) 2000, 2001, 2002-2006 Heikki Junnila, Stefan Krumm
  
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

#include "midi-inout.h"
#include "configuremidi-inout.h"
#include "../common/filehandler.h"
//#include "usbdmx.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <qthread.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <qapplication.h>
#include <assert.h>
#include <errno.h>
#include <linux/errno.h>

#define CONF_FILE "midi-inout.conf"

#define ID_CONFIGURE      10
#define ID_ACTIVATE       20


//@heikki: that lead to a nested loop with app.h including inputplugin.h
//#include "../../main/src/app.h"

//extern  App* _app;


static QMutex _mutex;

//
// Exported functions
//
extern "C" InputPlugin* create(t_plugin_id id)
{
    return new MidiInOut(id);
}

extern "C" void destroy(InputPlugin* object)
{
    delete object;
}


//
// the input thread class
//

void MidiInThread::run()
{
    unsigned int channel, command, value;
    struct timeval  timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    qDebug("Thread started: Midi Input");

    fd_set fs;
    FD_ZERO(&fs);
    FD_SET(m_device, &fs);

    QString txt;
    unsigned char buf[10]={999,999,999,999,999,999,999,999,999,999};
    QTime time;
    time.start();
    static int old_channel=0, old_value=0;

    do{
        //all the initializing stuff MUST be inside of the loop!!!!!!
        //else you will spend another half a day looking for wired errors!
        // SK         ;-)

        mutex.lock();

        int id = 0;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        fd_set fs;
        FD_ZERO(&fs);
        FD_SET(m_device, &fs);

        int i=select(m_device+1, &fs, NULL,NULL,NULL);

        if(i)
        {
            //msleep(50);
            buf[0]=999;
            buf[1]=999;
            buf[2]=999;

            read(m_device, buf, 1);//sizeof(buf));
            
            //txt.sprintf(" %d %d %d",buf[0],buf[1],buf[2]);
            //qDebug(txt);
            //InputEvent ie(0,buf[1],buf[2]);
            //txt.sprintf("***Midi sequence: %d %d %d",buf[0],buf[1],buf[2]);
            
            if(m_parent->debugLevel() == 2)
            {
                txt.sprintf("%u ",buf[0]);
                qDebug(txt);
            }

            // Extract midi command and channel from first byte


            // This is a workaround for Stefan's X11 Master Control Floorboard
            // which writes a chaotic command sequence on each button down/release
            if((240 & buf[0])==240)
            {
                msleep(100);
                //qDebug("X11 event!");
                read(m_device, &buf[0], 8);
                channel = buf[5] ^ 192;
                value = buf[6];
                id = 0;
                if(m_parent->debugLevel() == 1)
                {
                    txt.sprintf("X11 PC on channel %u    value = %u       %u     %u", channel, value, buf[8], buf[9]);
                    qDebug(txt);
                }
                //txt.sprintf("oldvalue %d   value %d    oldchannel %d  channel %d", old_value,value,old_channel,channel);
                //qDebug(txt);
                if(  old_value == value && old_channel == channel && time.elapsed()<1000)
                {
                    id = 999;
                    //qDebug("Same garbage..");
                }
                time.restart();
                old_value = value;
                old_channel = channel;
                channel = value;
                // Read the rest of garbage bytes...
                // read(m_device, &buf[0], 6);
            }else
                // This is a ControlChange event
                if((176 & buf[0])==176)
                {
                    read(m_device, &buf[1], 2);
                    channel = buf[0] ^ 176;
                    value = buf[2];
                    id = 1;
                    if(m_parent->debugLevel() == 1)
                    {
                        txt.sprintf("ControlChange event on Midi channel %u, controller = %u, value = %u", channel, buf[1],value);
                        qDebug(txt);
                    }
		    channel = buf[1];
                }else
                    // What about a ProgramChange?
                    if((192 & buf[0])==192)
                    {
                        read(m_device, &buf[1], 1);
                        channel = buf[0] ^ 192;
                        value = buf[1];
                        id = 0;
                        if(m_parent->debugLevel() == 1)
                        {
                            txt.sprintf("ProgramChange event on Midi channel %u,  value = %u", channel, value);
                            qDebug(txt);
                        }
                        channel = value;

                    }
                    else
                    {
                        if(m_parent->debugLevel() == 1)
                        {
                            txt.sprintf("Not supported midi sequence: %d %d %d",buf[0],buf[1],buf[2]);
                            qDebug(txt);
                        }
                    }

            InputEvent* ie = new InputEvent( id, channel, value );
            assert(m_parent->eventReceiver());
            QApplication::postEvent(m_parent->eventReceiver(), ie);
        }else
        {
            qDebug("Timeout on Midi device");
        }

        mutex.unlock();
        
    }while(1);

}

void MidiInThread::stop()
{
    mutex.lock();
    stopped = TRUE;
    terminate();
    qDebug("Thread stopped: Midi Input");
    mutex.unlock();
}

void MidiInThread::setDevice(int device)
{
    m_device  = device;
}

//
// Class implementation
//
MidiInOut::MidiInOut(t_plugin_id id) : InputPlugin(id)
{
    m_device = -1;
    m_name = QString("Midi Input-Output");
    m_type = InputType;
    m_version = 0x00010000;
    m_deviceName = QString("/dev/midi1");
    m_configDir = QString("~/.qlc/");
    m_inThread = new MidiInThread(this);
    //qDebug("Midi Input/Output created");
    //
    // @Heikki: is this a good place to have open() and activate() ?

    //open();
    //activate();

}

MidiInOut::~MidiInOut()
{
}

/* Attempt to open dmx device */
int MidiInOut::open()
{

    qDebug("Trying to open Midi In/Out!");
    if (m_device != -1)
    {
        qDebug("Midi Input/Output already open");
        return false;
    }

    m_device = ::open((const char*) m_deviceName, O_RDWR );//| O_NONBLOCK);
    if (m_device == -1)
    {
        perror("open");
        qDebug(" Midi Input/Output is not available at: " + m_deviceName);
    }
    else
    {
        qDebug("Opened Midi In/Out: " + m_deviceName);
        m_inThread->setDevice(m_device);
        m_inThread->start();
    }

    return errno;
}

int MidiInOut::close()
{
    qDebug("Closing Midi In/Out!");
    int r = 0;
    m_inThread->stop();
    r = ::close(m_device);
    if (r == -1)
    {
        perror("Closing midi ");
    }
    else
    {
        m_device = -1;
    }

    return r;
}

bool MidiInOut::isOpen()
{
    if (m_device == -1)
    {
        return false;
    }
    else
    {
        return true;
    }
}

int MidiInOut::configure()
{
    ConfigureMidiInOut* conf = new ConfigureMidiInOut(this);

    if (conf->exec() == QDialog::Accepted)
    {
        m_deviceName = conf->device();
        m_debugLevel = conf->debugLevel();
        saveSettings();
    }

    delete conf;

    return 0;
}

QString MidiInOut::infoText()
{
    QString t;
    QString str = QString::null;
    str += QString("<HTML><HEAD><TITLE>Plugin Info</TITLE></HEAD><BODY>");
    str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\"><TR>");
    str += QString("<TD BGCOLOR=\"black\"><FONT COLOR=\"white\" SIZE=\"5\">");
    str += name() + QString("</FONT></TD></TR></TABLE>");
    str += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
    str += QString("<TR>\n");
    str += QString("<TD><B>Version</B></TD>");
    str += QString("<TD>");
    t.setNum((version() >> 16) & 0xff);
    str += t + QString(".");
    t.setNum((version() >> 8) & 0xff);
    str += t + QString(".");
    t.setNum(version() & 0xff);
    str += t + QString("</TD>");
    str += QString("</TR>");

    str += QString("<TR>\n");
    str += QString("<TD><B>Status</B></TD>");
    str += QString("<TD>");
    if (isOpen() == true)
    {
        str += QString("<I>Active</I></TD>");
    }
    else
    {
        str += QString("Not Active</TD>");
    }
    str += QString("</TR>");

    str += QString("</TR>");
    str += QString("</TABLE>");
    str += QString("</BODY></HTML>");

    return str;
}

int MidiInOut::setConfigDirectory(QString dir)
{
    m_configDir = dir;
    return 0;
}

int MidiInOut::saveSettings()
{
    QString s;
    QString t;

    QString fileName = m_configDir + QString(CONF_FILE);
    qDebug(fileName);
    QFile file(fileName);

    if (file.open(IO_WriteOnly))
    {
        // Comment
        s = QString("# MidiInOut Plugin Configuration\n");
        file.writeBlock((const char*) s, s.length());

        // Entry type
        s = QString("Entry = ") + name() + QString("\n");
        file.writeBlock((const char*) s, s.length());

        s = QString("Device = ") + m_deviceName + QString("\n");
        file.writeBlock((const char*) s, s.length());

        file.close();
    }
    else
    {
        perror("file.open");
        qDebug("Unable to save MidiInOut configuration");
    }

    return errno;
}

int MidiInOut::loadSettings()
{
    QString fileName;
    QPtrList <QString> list;

    fileName = m_configDir + QString(CONF_FILE);

    FileHandler::readFileToList(fileName, list);

    for (QString* s = list.first(); s != NULL; s = list.next())
    {
        if (*s == QString("Entry"))
        {
            s = list.next();
            if (*s == name())
            {
                createContents(list);
            }
        }
    }

    return 0;
}

void MidiInOut::createContents(QPtrList <QString> &list)
{
    for (QString* s = list.next(); s != NULL; s = list.next())
    {
        if (*s == QString("Entry"))
        {
            s = list.prev();
            break;
        }
        else if (*s == QString("Device"))
        {
            m_deviceName = *(list.next());
        }
    }
}

void MidiInOut::contextMenu(QPoint pos)
{
    QPopupMenu* menu = new QPopupMenu();
    menu->insertItem("Configure...", ID_CONFIGURE);
    menu->insertSeparator();
    menu->insertItem("Activate", ID_ACTIVATE);

    connect(menu, SIGNAL(activated(int)),
            this, SLOT(slotContextMenuCallback(int)));
    menu->exec(pos, 0);
    delete menu;
}

void MidiInOut::slotContextMenuCallback(int item)
{
    switch(item)
    {
    case ID_CONFIGURE:
        configure();
        break;

    case ID_ACTIVATE:
        activate();
        break;

    default:
        break;
    }
}

void MidiInOut::activate()
{

    qDebug("Trying to activate midi inout!");

    open();
    if(!isOpen())
        qDebug("Device is not open but shall be activated");
    else
    {
        //  m_inThread->setDevice(m_device);
        //m_inThread->start();
        emit activated(this);
    }
}

//
// Write a value to a channel
//
int MidiInOut::writeChannel(t_channel channel, t_value value)
{
    /***int r = 0;
    unsigned char buf[512];

    _mutex.lock();

    m_values[channel] = value;

    ioctl(m_device, DMX_MEM_MAP_SET, DMX_TX_MEM);
    lseek(m_device, channel, SEEK_SET);
    r = write(m_device, &value, 1);
    if (r == -1)
      {
        perror("write channel");
      }
    *///

    /* read in for testing purpose
       ioctl(m_device, DMX_BLOCKING_SET, 0);
       ioctl(m_device, DMX_MEM_MAP_SET, DMX_RX_MEM);
       lseek(m_device, 0, SEEK_SET);   */

    /* read data */
    /*  read(m_device, buf, sizeof(buf));
      QString txt;
      
      txt.sprintf("Channel %d   value %d",channel, buf[channel]);
      qDebug(txt);

     */

    _mutex.unlock();

    return 0;
}

//
// Write num values starting from address
//
int MidiInOut::writeRange(t_channel address, t_value* values, t_channel num)
{
    /*
      assert(values);
      int r = 0;

      _mutex.lock();

      memcpy(m_values + address, values, num * sizeof(t_value));

      lseek(m_device, address, SEEK_SET);
      r = write(m_device, values, num);
      if (r == -1)
        {
          perror("write");
        }

      _mutex.unlock();
      return r;

    */
    return 0;
}

//
// Read a channel's value
//
int MidiInOut::readChannel(t_channel channel, t_value &value)
{
    _mutex.lock();
    value = m_values[channel];
    _mutex.unlock();

    return 0;
}

//
// Read num channel's values starting from address
//
int MidiInOut::readRange(t_channel address, t_value* values, t_channel num)
{
    assert(values);

    _mutex.lock();
    memcpy(values, m_values + address, num * sizeof(t_value));
    _mutex.unlock();

    return 0;
}


void MidiInOut::feedBack(int id, int channel, int value)
{

    QString txt;
    unsigned char buf[3]={0,0,0};
    if(id == 1)
    {
        buf[0] = (unsigned char) 176;
        buf[1] = (unsigned char) channel;
        buf[2] = (unsigned char) value;
    }
    //txt.sprintf("*** Feedback Midi sequence: %d %d %d",buf[0],buf[1],buf[2]);
    // qDebug(txt);
    int r = write(m_device, buf, 3);
    if (r == -1)
    {
        perror("write");
    }


}









