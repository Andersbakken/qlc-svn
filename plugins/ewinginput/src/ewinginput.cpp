/*
  Q Light Controller
  ewinginput.cpp

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

#include <QStringList>
#include <QUdpSocket>
#include <QDebug>

#include "eplaybackwing.h"
#include "eshortcutwing.h"
#include "eprogramwing.h"
#include "ewinginput.h"
#include "ewing.h"

/*****************************************************************************
 * EWingInput Initialization
 *****************************************************************************/

void EWingInput::init()
{
    /* Create a new UDP socket and start listening to packets coming to
       any local address. */
    m_socket = new QUdpSocket(this);
    reBindSocket();
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(slotReadSocket()));
}

EWingInput::~EWingInput()
{
    while (m_devices.isEmpty() == false)
        delete m_devices.takeFirst();
}

QString EWingInput::name()
{
    return QString("ENTTEC Wing Input");
}

void EWingInput::reBindSocket()
{
    if (m_socket->state() == QAbstractSocket::BoundState)
        m_socket->close();

    if (m_socket->bind(QHostAddress::Any, EWing::UDPPort) == false)
    {
        m_errorString = m_socket->errorString();
        qWarning() << Q_FUNC_INFO << m_errorString;
    }
    else
    {
        m_errorString.clear();
    }
}

/*****************************************************************************
 * Inputs
 *****************************************************************************/

void EWingInput::open(quint32 input)
{
    Q_UNUSED(input);
    reBindSocket();
}

void EWingInput::close(quint32 input)
{
    Q_UNUSED(input);
}

QStringList EWingInput::inputs()
{
    QStringList list;

    QListIterator <EWing*> it(m_devices);
    while (it.hasNext() == true)
        list << it.next()->name();

    return list;
}

QString EWingInput::infoText(quint32 input)
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
        str += tr("This plugin provides input support for Enttec Playback "
                  "and Enttec Shortcut Wings.");
        str += QString("</P>");

        if (m_socket->state() != QAbstractSocket::BoundState)
        {
            str += QString("<P>");
            str += tr("Unable to bind to UDP port %1:").arg(EWing::UDPPort);
            str += QString(" %1.").arg(m_errorString);
            str += QString("</P>");
        }
        else
        {
            str += QString("<P>");
            str += tr("Listening to UDP port %1.").arg(EWing::UDPPort);
            str += QString("</P>");
        }
    }
    else
    {
        /* A specific input line selected. Display its information if
           available. */
        EWing* dev = device(input);
        if (dev != NULL)
            str += dev->infoText();
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

EWing* EWingInput::createWing(QObject* parent, const QHostAddress& address,
                              const QByteArray& data)
{
    EWing* ewing;

    /* Check, that the message is from an ENTTEC Wing */
    if (EWing::isOutputData(data) == false)
        return NULL;

    switch (EWing::resolveType(data))
    {
    case EWing::Playback:
        ewing = new EPlaybackWing(parent, address, data);
        break;

    case EWing::Shortcut:
        ewing = new EShortcutWing(parent, address, data);
        break;

    case EWing::Program:
        ewing = new EProgramWing(parent, address, data);
        break;

    default:
        ewing = NULL;
        break;
    }

    return ewing;
}

EWing* EWingInput::device(const QHostAddress& address, EWing::Type type)
{
    QListIterator <EWing*> it(m_devices);
    while (it.hasNext() == true)
    {
        EWing* dev = it.next();
        if (dev->address() == address && dev->type() == type)
            return dev;
    }

    return NULL;
}

EWing* EWingInput::device(quint32 index)
{
    if (index < quint32(m_devices.count()))
        return m_devices.at(index);
    else
        return NULL;
}

static bool ewing_device_sort(const EWing* d1, const EWing* d2)
{
    /* Sort devices based on their addresses. Lexical sorting is enough. */
    return (d1->address().toString() < d2->address().toString());
}

void EWingInput::addDevice(EWing* device)
{
    Q_ASSERT(device != NULL);

    connect(device, SIGNAL(valueChanged(quint32,uchar)),
            this, SLOT(slotValueChanged(quint32,uchar)));

    m_devices.append(device);

    /* To maintain some persistency with the indices of multiple devices
       between sessions they need to be sorted according to some
       (semi-)permanent criteria. Their addresses shouldn't change too
       often, so let's use that. */
    qSort(m_devices.begin(), m_devices.end(), ewing_device_sort);

    emit configurationChanged();
}

void EWingInput::removeDevice(EWing* device)
{
    Q_ASSERT(device != NULL);
    m_devices.removeAll(device);
    delete device;

    emit configurationChanged();
}

void EWingInput::slotReadSocket()
{
    while (m_socket->hasPendingDatagrams() == true)
    {
        QHostAddress sender;
        QByteArray data;
        EWing* wing;

        /* Read data from socket */
        data.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(data.data(), data.size(), &sender);

        /* Check, whether we already have a device from this address */
        wing = device(sender, EWing::resolveType(data));
        if (wing == NULL)
        {
            /* New address. Create a new device. */
            wing = createWing(this, sender, data);
            if (wing != NULL)
                addDevice(wing);
        }
        else
        {
            // Since creating a wing already does parseData, don't do it again
            wing->parseData(data);
        }
    }
}

void EWingInput::slotValueChanged(quint32 channel, uchar value)
{
    EWing* wing = qobject_cast<EWing*> (QObject::sender());
    emit valueChanged(m_devices.indexOf(wing), channel, value);
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void EWingInput::configure()
{
    reBindSocket();
    emit configurationChanged();
}

bool EWingInput::canConfigure()
{
    return true;
}

/*****************************************************************************
 * Feedback
 *****************************************************************************/

void EWingInput::feedBack(quint32 input, quint32 channel, uchar value)
{
    EWing* ewing = device(input);
    if (ewing != NULL)
        ewing->feedBack(channel, value);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(ewinginput, EWingInput)
