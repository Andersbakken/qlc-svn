/*
  Q Light Controller
  enttecdmxusb.cpp

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,$
*/

#ifdef DBUS_ENABLED
#   include <QDBusConnection>
#endif

#include <QStringList>
#include <QMessageBox>
#include <QDebug>

#include "enttecdmxusbwidget.h"
#include "enttecdmxusbopen.h"
#include "enttecdmxusbpro.h"
#include "enttecdmxusbout.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBOut::~EnttecDMXUSBOut()
{
    while (m_widgets.isEmpty() == false)
        delete m_widgets.takeFirst();
}

void EnttecDMXUSBOut::init()
{
#ifdef DBUS_ENABLED
    /* Listen to device additions and removals thru DBus system bus */
    QDBusConnection::systemBus().connect(QString(),
                                         QString("/org/freedesktop/Hal/Manager"),
                                         QString("org.freedesktop.Hal.Manager"),
                                         QString("DeviceAdded"),
                                         this, SLOT(slotDeviceAdded(const QString&)));
    QDBusConnection::systemBus().connect(QString(),
                                         QString("/org/freedesktop/Hal/Manager"),
                                         QString("org.freedesktop.Hal.Manager"),
                                         QString("DeviceRemoved"),
                                         this, SLOT(slotDeviceRemoved(const QString&)));
#endif

    /* Search for new widgets */
    rescanWidgets();
}

QString EnttecDMXUSBOut::name()
{
    return QString("Enttec DMX USB Output");
}

#ifdef DBUS_ENABLED
void EnttecDMXUSBOut::slotDeviceAdded(const QString& name)
{
    QRegExp re("/org/freedesktop/Hal/devices/usb_device_403_*_*_if*_serial_usb_*");
    re.setPatternSyntax(QRegExp::Wildcard);
    if (name.contains(re) == true)
        rescanWidgets();
}

void EnttecDMXUSBOut::slotDeviceRemoved(const QString& name)
{
    QRegExp re("/org/freedesktop/Hal/devices/usb_device_403_*_*_if*_serial_usb_*");
    re.setPatternSyntax(QRegExp::Wildcard);
    if (name.contains(re) == true)
        rescanWidgets();
}
#endif

/****************************************************************************
 * Outputs
 ****************************************************************************/

void EnttecDMXUSBOut::open(quint32 output)
{
    if (output < quint32(m_widgets.size()))
        m_widgets.at(output)->open();
}

void EnttecDMXUSBOut::close(quint32 output)
{
    if (output < quint32(m_widgets.size()))
        m_widgets.at(output)->close();
}

QStringList EnttecDMXUSBOut::outputs()
{
    QStringList list;
    int i = 1;

    QListIterator <EnttecDMXUSBWidget*> it(m_widgets);
    while (it.hasNext() == true)
        list << QString("%1: %2").arg(i++).arg(it.next()->uniqueName());
    return list;
}

QString EnttecDMXUSBOut::infoText(quint32 output)
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
        if (m_widgets.size() == 0)
        {
            str += QString("<B>%1</B>").arg(tr("No devices available."));
            str += QString("<P>");
            str += tr("Make sure that you have your hardware firmly plugged in. "
                      "NOTE: FTDI VCP interface is not supported by this plugin.");
            str += QString("</P>");
        }

        str += QString("<P>");
        str += tr("This plugin provides DMX output support for");
        str += QString(" DMXKing USB DMX512-A, Enttec DMX USB Pro, "
                       "Enttec Open DMX USB, FTDI USB COM485 Plus1 ");
        str += tr("and compatible devices.");
        str += QString("</P>");
    }
    else if (output < quint32(m_widgets.size()))
    {
        str += QString("<H3>%1</H3>").arg(outputs()[output]);
        str += QString("<P>");
        str += tr("Device is operating correctly.");
        str += QString("</P>");
        QString add = m_widgets[output]->additionalInfo();
        if (add.isEmpty() == false)
            str += add;
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void EnttecDMXUSBOut::outputDMX(quint32 output, const QByteArray& universe)
{
    if (output < quint32(m_widgets.size()))
        m_widgets.at(output)->sendDMX(universe);
}

/****************************************************************************
 * Configuration
 ****************************************************************************/

void EnttecDMXUSBOut::configure()
{
    int r = QMessageBox::question(NULL, name(),
                                  tr("Do you wish to re-scan your hardware?"),
                                  QMessageBox::Yes, QMessageBox::No);
    if (r == QMessageBox::Yes)
        rescanWidgets();
}

bool EnttecDMXUSBOut::canConfigure()
{
    return true;
}

/****************************************************************************
 * Enttec Widgets
 ****************************************************************************/

bool EnttecDMXUSBOut::rescanWidgets()
{
    while (m_widgets.isEmpty() == false)
        delete m_widgets.takeFirst();
    m_widgets = QLCFTDI::widgets();
    emit configurationChanged();
    return true;
}

/****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(enttecdmxusbout, EnttecDMXUSBOut)
