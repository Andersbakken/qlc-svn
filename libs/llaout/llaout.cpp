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


/*
 * Start the plugin. It's hard to say if we want LLA running if there aren't
 * any output universes active.
 */
void LLAOut::init()
{
  //m_thread = new LlaEmbeddedServer();
  m_thread = new LlaStandaloneClient();
  if (!m_thread->start())
  {
    qWarning() << "llaout: start thread failed";
    return;
  }
  memset(m_dmx_data, 0x00, K_UNIVERSE_COUNT * sizeof(t_channel*));
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

  if (m_dmx_data[output])
  {
    qWarning() << "llaout: output " << output << " already active";
    return;
  }

  t_channel *dmx_buffer = (t_channel*) malloc(K_UNIVERSE_SIZE *
                                              sizeof(t_channel));

  if (!dmx_buffer)
  {
    qWarning() << "llaout: malloc failed for output " << output;
    return;
  }

  m_dmx_data[output] = dmx_buffer;
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

  if (!m_dmx_data[output])
  {
    qWarning() << "llaout: output " << output << " not active";
    return;
  }
  free(m_dmx_data[output]);
  m_dmx_data[output] = NULL;
}


/*
 * Return a list of our outputs. For now we output on LLA universes 1 to
 * K_UNIVERSE_COUNT.
 */
QStringList LLAOut::outputs()
{
  QStringList list;
  for (unsigned int i = 1; i <= K_UNIVERSE_COUNT; ++i)
  {
    QString s;
    s.sprintf("%d: LLA Universe %d", i, i);
    list << s;
  }
  return list;
}


/*
 * The plugin name
 */
QString LLAOut::name()
{
  return QString("LLA Output");
}


/*
 * Configure this plugin.
 * TODO: Add this.
 * Things we may want:
 *   - http server on/off
 *   - listen for other clients
 *   - universe ID
 *   - lla device patching
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
 * Write a single channel.
 */
void LLAOut::writeChannel(t_output output, t_channel channel, t_value value)
{
  if (output > K_UNIVERSE_COUNT || !m_thread || channel > K_UNIVERSE_SIZE)
    return;

  m_dmx_data[output][channel] = value;
}


/*
 * Write a range of channels.
 */
void LLAOut::writeRange(t_output output, t_channel address, t_value* values,
                        t_channel num)
{
  if (output > K_UNIVERSE_COUNT || !m_thread || address > K_UNIVERSE_SIZE)
    return;

  unsigned int length = num;
  if (address + length > K_UNIVERSE_SIZE)
    length = K_UNIVERSE_SIZE - address;

  memcpy(&m_dmx_data[output][address], values, length);
  m_thread->write_dmx(output + 1, (dmx_t*) m_dmx_data[output], K_UNIVERSE_SIZE);
}


/*
 * Read a single channel.
 */
void LLAOut::readChannel(t_output output, t_channel channel, t_value* value)
{
  if (output > K_UNIVERSE_COUNT || !m_thread || channel > K_UNIVERSE_SIZE)
    return;

  *value = m_dmx_data[output][channel];
}


/*
 * Read a change of channels.
 */
void LLAOut::readRange(t_output output, t_channel address, t_value* values,
                       t_channel num)
{
  if (output > K_UNIVERSE_COUNT || !m_thread || address > K_UNIVERSE_SIZE)
    return;

  unsigned int length = num;
  if (address + length > K_UNIVERSE_SIZE)
    length = K_UNIVERSE_SIZE - address;

  memcpy(values, &m_dmx_data[output][address], length);
}

Q_EXPORT_PLUGIN2(llaout, LLAOut)
