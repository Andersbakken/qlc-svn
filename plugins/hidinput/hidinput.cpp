/*
  Q Light Controller
  hidinput.cpp

  Copyright (c) Heikki Junnila

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

#include <QApplication>
#include <QMessageBox>
#include <QStringList>
#include <QDebug>
#include <QDir>

#include "configurehidinput.h"
#include "hidjsdevice.h"
#include "hidpoller.h"
#include "hidinput.h"

/*****************************************************************************
 * HIDInputEvent
 *****************************************************************************/

static const QEvent::Type _HIDInputEventType = static_cast<QEvent::Type>
        (QEvent::registerEventType());

HIDInputEvent::HIDInputEvent(HIDDevice* device, quint32 input,
                             quint32 channel, uchar value,
                             bool alive) : QEvent(_HIDInputEventType)
{
    m_device = device;
    m_input = input;
    m_channel = channel;
    m_value = value;
    m_alive = alive;
}

HIDInputEvent::~HIDInputEvent()
{
}

/*****************************************************************************
 * HIDInput Initialization
 *****************************************************************************/

void HIDInput::init()
{
    m_poller = new HIDPoller(this);
    rescanDevices();
}

HIDInput::~HIDInput()
{
    while (m_devices.isEmpty() == false)
        delete m_devices.takeFirst();

    m_poller->stop();
    delete m_poller;
}

QString HIDInput::name()
{
    return QString("HID Input");
}

QStringList HIDInput::inputs()
{
    QStringList list;

    QListIterator <HIDDevice*> it(m_devices);
    while (it.hasNext() == true)
        list << it.next()->name();

    return list;
}

/*****************************************************************************
 * Inputs
 *****************************************************************************/

void HIDInput::open(quint32 input)
{
    HIDDevice* dev = device(input);
    if (dev != NULL)
        addPollDevice(dev);
    else
        qDebug() << name() << "has no input number:" << input;
}

void HIDInput::close(quint32 input)
{
    HIDDevice* dev = device(input);
    if (dev != NULL)
        removePollDevice(dev);
    else
        qDebug() << name() << "has no input number:" << input;
}

void HIDInput::customEvent(QEvent* event)
{
    if (event->type() == _HIDInputEventType)
    {
        HIDInputEvent* e = static_cast<HIDInputEvent*> (event);
        if (e != NULL && e->m_alive == true)
            emit valueChanged(e->m_input, e->m_channel, e->m_value);
        else
            removeDevice(e->m_device);

        event->accept();
    }
}

QString HIDInput::infoText(quint32 input)
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<H3>%1</H3>").arg(name());

    if (input == KInputInvalid)
    {
        /* Plugin or just an invalid input selected. Display generic
           information. */
        str += QString("<P>");
        str += tr("This plugin provides input support for HID-based joysticks.");
        str += QString("</P>");
    }
    else
    {
        /* A specific input line selected. Display its information if
           available. */
        HIDDevice* dev = device(input);
        if (dev != NULL)
            str += dev->infoText();
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void HIDInput::configure()
{
    ConfigureHIDInput conf(NULL, this);
    conf.exec();
}

bool HIDInput::canConfigure()
{
    return true;
}

/*****************************************************************************
 * Feedback
 *****************************************************************************/

void HIDInput::feedBack(quint32 input, quint32 channel, uchar value)
{
    Q_UNUSED(input);
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void HIDInput::rescanDevices()
{
    quint32 line = 0;

    /* Copy the pointers from our devices list into a list of devices
       to destroy in case some of them have disappeared. */
    QList <HIDDevice*> destroyList(m_devices);

    /* Check all files matching filter "/dev/input/js*" */
    QDir dir("/dev/input/", QString("js*"), QDir::Name, QDir::System);
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        /* Construct an absolute path for the file */
        QString path(dir.absoluteFilePath(it.next()));

        /* Check that we can at least read from the device. Otherwise
           deem it to ge destroyed. */
        if (QFile::permissions(path) & QFile::ReadOther)
        {
            HIDDevice* dev = device(path);
            if (dev == NULL)
            {
                /* This device is unknown to us. Add it. */
                dev = new HIDJsDevice(this, line++, path);
                addDevice(dev);
            }
            else
            {
                /* Remove the device from our destroy list,
                   since it is still available */
                destroyList.removeAll(dev);
            }
        }
        else
        {
            /* The file is not readable. If we have an entry for
               it, it must be destroyed. */
            HIDDevice* dev = device(path);
            if (dev != NULL)
                removeDevice(dev);
        }
    }

    /* Destroy all devices that were not found during rescan */
    while (destroyList.isEmpty() == false)
        removeDevice(destroyList.takeFirst());
}

HIDDevice* HIDInput::device(const QString& path)
{
    QListIterator <HIDDevice*> it(m_devices);

    while (it.hasNext() == true)
    {
        HIDDevice* dev = it.next();
        if (dev->path() == path)
            return dev;
    }

    return NULL;
}

HIDDevice* HIDInput::device(quint32 index)
{
    if (index < quint32(m_devices.count()))
        return m_devices.at(index);
    else
        return NULL;
}

void HIDInput::addDevice(HIDDevice* device)
{
    Q_ASSERT(device != NULL);

    m_devices.append(device);
    emit deviceAdded(device);

    emit configurationChanged();
}

void HIDInput::removeDevice(HIDDevice* device)
{
    Q_ASSERT(device != NULL);

    removePollDevice(device);
    m_devices.removeAll(device);

    emit deviceRemoved(device);
    delete device;

    emit configurationChanged();
}

/*****************************************************************************
 * Device poller
 *****************************************************************************/

void HIDInput::addPollDevice(HIDDevice* device)
{
    Q_ASSERT(device != NULL);
    m_poller->addDevice(device);
}

void HIDInput::removePollDevice(HIDDevice* device)
{
    Q_ASSERT(device != NULL);
    m_poller->removeDevice(device);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(hidinput, HIDInput)
