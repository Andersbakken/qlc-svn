/*
  Q Light Controller
  llaoutthread.cpp

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

#include <QDebug>
#include "llaoutthread.h"


/*
 * Called when there is data to be read on the socket.
 * @param socket the socket with new data.
 */
int PipeListener::SocketReady(lla::select_server::ConnectedSocket *socket) {
  unsigned int data_read;
  int ret = socket->Receive((uint8_t*) &m_data, sizeof(m_data), data_read);
  if (ret < 0)
  {
    qCritical() << "llaout: socket receive failed";
    return 0;
  }
  if (!m_client->SendDmx(m_data.universe, m_data.data,
                         data_read - sizeof(m_data.universe)))
    qWarning() << "llaout:: SendDmx() failed";
  return 0;
}


/*
 * Clean up.
 */
LlaOutThread::~LlaOutThread()
{
  if (m_client)
  {
    m_client->Stop();
    delete m_client;
  }

  if (m_pipe)
    delete m_pipe;

  if (m_listener)
    delete m_listener;
}


/*
 * Start the LLA thread
 *
 * @return true if sucessfull, false otherwise
 */
bool LlaOutThread::start(Priority priority)
{
  if (!init())
    return false;

  if (!m_pipe)
  {
    // setup the pipe to recv dmx data on
    m_pipe = new lla::select_server::LoopbackSocket();
    m_pipe->Init();
    m_listener = new PipeListener(m_client);
    m_pipe->SetListener(m_listener);
    // TODO add the manager here
    // the manager needs to delete it as well
    m_ss->AddSocket(m_pipe);
  }

  QThread::start(priority);
  return true;
}


/*
 * Close the socket which stops the thread.
 */
void LlaOutThread::stop()
{
  m_pipe->Close();
  return;
}


/*
 * Run the select server.
 */
void LlaOutThread::run()
{
  m_ss->Run();
  return;
}


/*
 * Send the new data over the socket so that the other thread picks it up.
 * @param universe the universe nmuber this data is for
 * @param data a pointer to the data
 * @param channels the number of channels
 */
int LlaOutThread::write_dmx(unsigned int universe, dmx_t *data,
                            unsigned int channels)
{
  unsigned int len = channels < K_UNIVERSE_SIZE ? channels : K_UNIVERSE_SIZE;
  m_data.universe = universe;
  memcpy(m_data.data, data, len);
  m_pipe->Send((uint8_t*) &m_data, sizeof(m_data));
  return 0;
}


/*
 * Setup the LlaClient to communicate with the server.
 * @return true if the setup worked corectly.
 */
bool LlaOutThread::setup_client(lla::select_server::ConnectedSocket *socket) {
  if (!m_client)
  {
    m_client = new lla::LlaClient(socket);
    if (!m_client->Setup())
    {
      qWarning() << "llaout: client setup failed";
      delete m_client;
      m_client = NULL;
      return false;
    }
    m_ss->AddSocket(socket);
  }
  return true;
}


/*
 * Clean up the standalone client.
 */
LlaStandaloneClient::~LlaStandaloneClient()
{
  if (m_ss)
    delete m_ss;

  if (m_tcp_socket)
  {
    delete m_tcp_socket;
    m_tcp_socket = NULL;
  }
}


/*
 * Setup the standalone client.
 * @return true is successful.
 */
bool LlaStandaloneClient::init()
{
  if (m_init_run)
    return true;

  if (!m_ss)
    m_ss = new lla::select_server::SelectServer();

  if (!m_tcp_socket)
  {
    m_tcp_socket = new lla::select_server::TcpSocket();
    if (!m_tcp_socket->Connect("127.0.0.1", LLA_DEFAULT_PORT))
    {
      qWarning() << "llaout: Connect failed, is LLAD running?";
      delete m_tcp_socket;
      m_tcp_socket = NULL;
      delete m_ss;
      m_ss = NULL;
      return false;
    }
  }

  if (!setup_client(m_tcp_socket))
  {
    m_tcp_socket->Close();
    delete m_tcp_socket;
    m_tcp_socket = NULL;
    delete m_ss;
    m_ss = NULL;
    return false;
  }
  m_init_run = true;
  return true;
}


/*
 * Clean up the embedded server.
 */
LlaEmbeddedServer::~LlaEmbeddedServer()
{

  if (m_daemon)
    delete m_daemon;

  if (m_pipe_socket)
    delete m_pipe_socket;
}


/*
 * Setup the embedded server.
 * @return true is successful.
 */
bool LlaEmbeddedServer::init()
{
  if (m_init_run)
    return true;

  lla::lla_server_options options;
  options.http_enable = true;
  m_daemon = new lla::LlaDaemon(options, NULL);
  if (!m_daemon->Init())
  {
    qWarning() << "LLA Server failed init";
    delete m_daemon;
    m_daemon = NULL;
    return false;
  }
  m_ss = m_daemon->GetSelectServer();

  // setup the pipe socket used to communicate with the LlaServer
  if (!m_pipe_socket)
  {
    m_pipe_socket = new lla::select_server::PipeSocket();
    if (!m_pipe_socket->Init())
    {
      qWarning() << "llaout: pipe failed";
      delete m_pipe_socket;
      m_pipe_socket = NULL;
      delete m_daemon;
      m_daemon = NULL;
      return false;
    }
    m_ss->AddSocket(m_pipe_socket);
  }

  if (!setup_client(m_pipe_socket))
  {
      delete m_pipe_socket;
      m_pipe_socket = NULL;
      delete m_daemon;
      m_daemon = NULL;
      return false;
  }

  m_daemon->GetLlaServer()->NewConnection(m_pipe_socket->OppositeEnd());
  m_init_run = true;
  return true;
}
