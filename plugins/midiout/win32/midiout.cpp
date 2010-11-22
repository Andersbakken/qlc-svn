/*
  Q Light Controller
  midiout.cpp

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

#include <QStringList>
#include <windows.h>
#include <QDebug>

#include "configuremidiout.h"
#include "mididevice.h"
#include "midiout.h"

/*****************************************************************************
 * MIDIOut Initialization
 *****************************************************************************/

MIDIOut::~MIDIOut()
{
    /* Delete all MIDI devices. */
    while (m_devices.isEmpty() == false)
        delete m_devices.takeFirst();
}

void MIDIOut::init()
{
    /* Find out what devices we have */
    rescanDevices();
}

QString MIDIOut::name()
{
    return QString("MIDI Output");
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

void MIDIOut::open(quint32 output)
{
    MIDIDevice* dev = device(output);
    if (dev != NULL)
        dev->open();
    else
        qDebug() << name() << "has no output number:" << output;
}

void MIDIOut::close(quint32 output)
{
    MIDIDevice* dev = device(output);
    if (dev != NULL)
        dev->open();
    else
        qDebug() << name() << "has no output number:" << output;
}

QStringList MIDIOut::outputs()
{
    QStringList list;
    int i = 1;

    QListIterator <MIDIDevice*> it(m_devices);
    while (it.hasNext() == true)
        list << QString("%1: %2").arg(i++).arg(it.next()->name());

    return list;
}

QString MIDIOut::infoText(quint32 output)
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
        str += tr("This plugin provides DMX output support for various MIDI devices.");
        str += QString("</P>");
    }
    else
    {
        MIDIDevice* dev = device(output);
        if (dev != NULL)
            str += device(output)->infoText();
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void MIDIOut::outputDMX(quint32 output, const QByteArray& universe)
{
    MIDIDevice* dev = device(output);
    if (dev != NULL)
        dev->outputDMX(universe);
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void MIDIOut::configure()
{
    ConfigureMIDIOut cmo(NULL, this);
    if (cmo.exec() == QDialog::Accepted)
        emit configurationChanged();
}

bool MIDIOut::canConfigure()
{
    return true;
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void MIDIOut::rescanDevices()
{
    UINT deviceCount;

    /* Destroy existing devices in case something has changed */
    while (m_devices.isEmpty() == false)
        removeDevice(m_devices.takeFirst());

    /* Create devices for each valid midi input */
    deviceCount = midiOutGetNumDevs();
    for (UINT id = 0; id < deviceCount; id++)
        addDevice(new MIDIDevice(this, id));
}

MIDIDevice* MIDIOut::device(unsigned int index)
{
    if (index < static_cast<unsigned int> (m_devices.size()))
        return m_devices.at(index);
    else
        return NULL;
}

void MIDIOut::addDevice(MIDIDevice* device)
{
    Q_ASSERT(device != NULL);

    m_devices.append(device);
    emit deviceAdded(device);
}

void MIDIOut::removeDevice(MIDIDevice* device)
{
    Q_ASSERT(device != NULL);

    device->close();
    m_devices.removeAll(device);
    emit deviceRemoved(device);

    delete device;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(midiout, MIDIOut)
