/*
  Q Light Controller
  olaout.cpp

  Copyright (c) Heikki Junnila
                Simon Newton

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
#include <QString>
#include <QDebug>
#include <QSettings>

#include "configureolaout.h"
#include "olaout.h"
#include "qlclogdestination.h"

OLAOut::~OLAOut()
{
    if (m_thread != NULL)
    {
        m_thread->stop();
        delete m_thread;
    }
    if (m_log_destination)
    {
        ola::InitLogging(ola::OLA_LOG_WARN, NULL);
        delete m_log_destination;
    }
}

/*
 * Start the plugin. It's hard to say if we want OLA running if there aren't
 * any output universes active.
 */
void OLAOut::init()
{
    m_embedServer = false;
    m_thread = NULL;
    m_log_destination = new ola::QLCLogDestination();
    ola::InitLogging(ola::OLA_LOG_WARN, m_log_destination);

    // TODO: load this from a savefile at some point
    for (unsigned int i = 1; i <= K_UNIVERSE_COUNT; ++i)
        m_output_list.append(i);

    QSettings settings;
    bool es = settings.value("OLAOut/embedded").toBool();
    // Make sure the thread is started the first time round
    m_embedServer = !es;
    // This should load from the settings when it is made
    setServerEmbedded(es);
}

/*
 * Is the plugin currently running as a stand alone daemon
 */
bool OLAOut::isServerEmbedded()
{
    return m_embedServer;
}

/*
 * Set whether or not to run as a standalone daemon
 */
void OLAOut::setServerEmbedded(bool embedServer)
{
    if (embedServer != m_embedServer) {
        if (m_thread != NULL)
        {
            m_thread->stop();
            delete m_thread;
        }
        m_embedServer = embedServer;
        if (m_embedServer)
        {
            qWarning() << "olaout: running as embedded";
            m_thread = new OlaEmbeddedServer();
        } else
        {
            m_thread = new OlaStandaloneClient();
        }
        if (!m_thread->start())
        {
            qWarning() << "olaout: start thread failed";
        }
        QSettings settings;
        settings.setValue("OLAOut/embedded", m_embedServer);
    }
}

/*
 * Open a universe for output
 * @param output the universe id
 */
void OLAOut::open(quint32 output)
{
    if (output >= K_UNIVERSE_COUNT)
    {
        qWarning() << "olaout: output " << output << " out of range";
        return;
    }
}


/*
 * Close this universe.
 * @param output the universe id
 */
void OLAOut::close(quint32 output)
{
    if (output >= K_UNIVERSE_COUNT)
    {
        qWarning() << "olaout: output " << output << " out of range";
        return;
    }
}


/*
 * Return a list of our outputs. For now we output on OLA universes 1 to
 * K_UNIVERSE_COUNT.
 */
QStringList OLAOut::outputs()
{
    QStringList list;
    for (int i = 0; i < m_output_list.size(); ++i)
        list << QString("%1: OLA Universe %1").arg(i + 1);
    return list;
}


/*
 * The plugin name
 */
QString OLAOut::name()
{
    return QString("OLA Output");
}


/*
 * Configure this plugin.
 * TODO: Add this.
 * Things we may want:
 *  - http server on/off
 *  - listen for other clients
 *  - universe ID
 *  - ola device patching
 */
void OLAOut::configure()
{
    ConfigureOLAOut conf(NULL, this);
    if (conf.exec() == QDialog::Accepted)
        emit configurationChanged();
}

bool OLAOut::canConfigure()
{
    return true;
}

/*
 * The plugin description.
 */
QString OLAOut::infoText(quint32 output)
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
        str += tr("This plugin provides DMX output support for the Open Lighting Architecture (OLA).");
        str += QString("</P>");
    }
    else
    {
        str += QString("<H3>%1</H3>").arg(outputs()[output]);
        str += QString("<P>");
        str += tr("This is the output for OLA universe %1").arg(output + 1);
        str += QString("</P>");
    }

    str += QString("</BODY>");
    str += QString("</HTML>");
    return str;
}


void OLAOut::outputDMX(quint32 output, const QByteArray& universe)
{
    if (output > K_UNIVERSE_COUNT || !m_thread)
        return;

    m_thread->write_dmx(m_output_list[output], universe);
}

/*
 * Return the output: universe mapping
 */
const OutputList OLAOut::outputMapping() const
{
    return m_output_list;
}


/*
 * Set the OLA universe for an output
 * @param output the id of the output to change
 * @param universe the OLA universe id
 */
void OLAOut::setOutputUniverse(quint32 output, unsigned int universe)
{
    if (output > K_UNIVERSE_COUNT)
        return;
    m_output_list[output] = universe;
}

Q_EXPORT_PLUGIN2(olaout, OLAOut)
