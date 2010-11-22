/*
  Q Light Controller
  midiinput.cpp

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

#include <QApplication>
#include <QStringList>
#include <Windows.h>
#include <QPalette>
#include <QDebug>

#include "configuremidiinput.h"
#include "mididevice.h"
#include "midiinput.h"

/*****************************************************************************
 * MIDIInput Initialization
 *****************************************************************************/

void MIDIInput::init()
{
    rescanDevices();
}

MIDIInput::~MIDIInput()
{
    while (m_devices.isEmpty() == false)
        delete m_devices.takeFirst();
}

QString MIDIInput::name()
{
    return QString("MIDI Input");
}

/*****************************************************************************
 * Inputs
 *****************************************************************************/

void MIDIInput::open(quint32 input)
{
    MIDIDevice* dev = device(input);
    if (dev != NULL)
    {
        connect(dev, SIGNAL(valueChanged(quint32, uchar)),
                this, SLOT(slotDeviceValueChanged(quint32, uchar)));

        dev->open();
    }
    else
    {
        qDebug() << name() << "has no input number:" << input;
    }
}

void MIDIInput::close(quint32 input)
{
    MIDIDevice* dev = device(input);
    if (dev != NULL)
    {
        disconnect(dev, SIGNAL(valueChanged(quint32, uchar)),
                   this, SLOT(slotDeviceValueChanged(quint32, uchar)));

        dev->close();
    }
    else
    {
        qDebug() << name() << "has no input number:" << input;
    }
}

QStringList MIDIInput::inputs()
{
    QStringList list;

    QListIterator <MIDIDevice*> it(m_devices);
    while (it.hasNext() == true)
        list << it.next()->name();

    return list;
}

QString MIDIInput::infoText(quint32 input)
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    if (input == KInputInvalid)
    {
        str += QString("<H3>%1</H3>").arg(name());
        str += QString("<P>");
        str += tr("This plugin provides input support for various MIDI devices.");
        str += QString("</P>");
    }
    else
    {
        MIDIDevice* dev = device(input);
        if (dev != NULL)
            str += device(input)->infoText();
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void MIDIInput::configure()
{
    ConfigureMIDIInput cmi(NULL, this);
    cmi.exec();
}

bool MIDIInput::canConfigure()
{
    return true;
}

/*****************************************************************************
 * Feedback
 *****************************************************************************/

void MIDIInput::feedBack(quint32 input, quint32 channel, uchar value)
{
    MIDIDevice* dev = device(input);
    if (dev != NULL)
        dev->feedBack(channel, value);
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void MIDIInput::rescanDevices()
{
    UINT deviceCount;

    /* Destroy existing devices in case something has changed */
    while (m_devices.isEmpty() == false)
        removeDevice(m_devices.takeFirst());

    /* Create devices for each valid midi input */
    deviceCount = midiInGetNumDevs();
    for (UINT id = 0; id < deviceCount; id++)
        addDevice(new MIDIDevice(this, id));
}

MIDIDevice* MIDIInput::device(quint32 index)
{
    if (index < static_cast<quint32> (m_devices.count()))
        return m_devices.at(index);
    else
        return NULL;
}

void MIDIInput::addDevice(MIDIDevice* device)
{
    Q_ASSERT(device != NULL);

    m_devices.append(device);
    emit deviceAdded(device);

    emit configurationChanged();
}

void MIDIInput::removeDevice(MIDIDevice* device)
{
    Q_ASSERT(device != NULL);

    device->close();
    m_devices.removeAll(device);
    emit deviceRemoved(device);
    delete device;

    emit configurationChanged();
}

void MIDIInput::slotDeviceValueChanged(quint32 channel, uchar value)
{
    MIDIDevice* device = qobject_cast<MIDIDevice*> (QObject::sender());
    if (device == NULL)
        return;

    quint32 input = m_devices.indexOf(device);
    emit valueChanged(input, channel, value);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(midiinput, MIDIInput)
