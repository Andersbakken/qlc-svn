/*
  Q Light Controller
  llaout.cpp

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

#include "common/qlcfile.h"
#include "configurellaout.h"
#include "llaout.h"


LLAOut::LLAOut()
{
  m_embedServer = false;
  m_thread = NULL;
}

LLAOut::~LLAOut() {
  if (m_thread != NULL) {
    m_thread->stop();
    delete m_thread;
  }
  m_thread = NULL;
}

/*
 * Start the plugin. It's hard to say if we want LLA running if there aren't
 * any output universes active.
 */
void LLAOut::init()
{
  // TODO: load this from a savefile at some point
  for (unsigned int i = 1; i <= K_UNIVERSE_COUNT; ++i)
    m_output_list.append(i);

  bool es = false;
  // Make sure the thread is started the first time round
  m_embedServer = !es;
  // This should load from the settings when it is made
  setServerEmbedded(es);
}

/*
 * Is the plugin currently running as a stand alone daemon
 */
bool LLAOut::isServerEmbedded()
{
  return m_embedServer;
}

/*
 * Set whether or not to run as a standalone daemon
 */
void LLAOut::setServerEmbedded(bool embedServer)
{
  if (embedServer != m_embedServer) {
    if (m_thread != NULL) {
      m_thread->stop();
      delete m_thread;
    }
    m_embedServer = embedServer;
    if (m_embedServer) {
      m_thread = new LlaEmbeddedServer();
    } else {
      m_thread = new LlaStandaloneClient();
    }
    if (!m_thread->start())
    {
      qWarning() << "llaout: start thread failed";
    }
    // TODO: Save the value to the configuration
  }
}

/*
 * Open a universe for output
 * @param output the universe id
 */
void LLAOut::open(t_output output)
{
  if (output >= K_UNIVERSE_COUNT)
  {
    qWarning() << "llaout: output " << output << " out of range";
    return;
  }
}


/*
 * Close this universe.
 * @param output the universe id
 */
void LLAOut::close(t_output output)
{
  if (output >= K_UNIVERSE_COUNT)
  {
    qWarning() << "llaout: output " << output << " out of range";
    return;
  }
}


/*
 * Return a list of our outputs. For now we output on LLA universes 1 to
 * K_UNIVERSE_COUNT.
 */
QStringList LLAOut::outputs()
{
  QStringList list;
  for (int i = 0; i != m_output_list.size(); ++i)
  {
    QString s;
    s.sprintf("LLA Output %d", i+1);
    list << s;
  }
  return list;
}


/*
 * The plugin name
 */
QString LLAOut::name()
{
  return QString("LLA Output Plugin");
}


/*
 * Configure this plugin.
 * TODO: Add this.
 * Things we may want:
 *  - http server on/off
 *  - listen for other clients
 *  - universe ID
 *  - lla device patching
 */
void LLAOut::configure()
{
  ConfigureLLAOut conf(NULL, this);
  conf.exec();
}


/*
 * The plugin description.
 */
QString LLAOut::infoText(t_output output)
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
    str += QString("the Linux Lighting Architecture (LLA). ");
    str += QString("See <a href=\"http://www.nomis52.net\">");
    str += QString("http://www.nomis52.net</a> for more ");
    str += QString("information.");
    str += QString("</P>");
  }
  else
  {
    str += QString("<H3>%1</H3>").arg(outputs()[output]);
    str += QString("<P>");
    str += QString("This is the output for LLA universe %1").arg(output + 1);
    str += QString("</P>");
  }

  str += QString("</BODY>");
  str += QString("</HTML>");
  return str;
}


/*
 * Write a single channel. Heikki says this will be deprecated soon.
 */
void LLAOut::writeChannel(t_output output, t_channel channel, t_value value)
{
  Q_UNUSED(output);
  Q_UNUSED(channel);
  Q_UNUSED(value);
}


/*
 * Write a range of channels. We can assume here that address is always 0 and
 * num is always 512.
 */
void LLAOut::writeRange(t_output output, t_channel address, t_value* values,
                        t_channel num)
{
  if (output > K_UNIVERSE_COUNT || !m_thread || address != 0 ||
      num != K_UNIVERSE_SIZE)
    return;
  m_thread->write_dmx(m_output_list[output], (dmx_t*) values, num);
}


/*
 * Read a single channel. Heikki says this will be deprecated soon.
 */
void LLAOut::readChannel(t_output output, t_channel channel, t_value* value)
{
  Q_UNUSED(output);
  Q_UNUSED(channel);
  Q_UNUSED(value);
}


/*
 * Read a change of channels. Heikki says this will be deprecated soon.
 */
void LLAOut::readRange(t_output output, t_channel address, t_value* values,
                       t_channel num)
{
  Q_UNUSED(output);
  Q_UNUSED(address);
  Q_UNUSED(values);
  Q_UNUSED(num);
}


/*
 * Return the output: universe mapping
 */
const OutputList LLAOut::outputMapping() const
{
  return m_output_list;
}


/*
 * Set the LLA universe for an output
 * @param output the id of the output to change
 * @param universe the LLA universe id
 */
void LLAOut::setOutputUniverse(t_output output, unsigned int universe)
{
  if (output > K_UNIVERSE_COUNT)
    return;
  m_output_list[output] = universe;
}

Q_EXPORT_PLUGIN2(llaout, LLAOut)
