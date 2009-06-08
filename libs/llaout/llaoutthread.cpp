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
#include <lla/Closure.h>
#include "llaoutthread.h"


/*
 * Clean up.
 */
LlaOutThread::~LlaOutThread()
{
  wait();
  if (m_client)
  {
    m_client->Stop();
    delete m_client;
  }

  if (m_pipe)
    delete m_pipe;

  cleanup();
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
    m_pipe = new lla::network::LoopbackSocket();
    m_pipe->Init();

    m_pipe->SetOnData(lla::NewClosure(this, &LlaOutThread::new_pipe_data));
    m_pipe->SetOnClose(lla::NewSingleClosure(this, &LlaOutThread::pipe_closed));
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
  if (m_pipe)
    m_pipe->CloseClient();
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
  if (m_pipe)
    m_pipe->Send((uint8_t*) &m_data, sizeof(m_data));
  return 0;
}


/*
 * Called when the pipe used to communicate between QLC and LLA is closed
 */
int LlaOutThread::pipe_closed() {
  // We don't need to delete the socket here because that gets done in the
  // Destructor.
  m_ss->Terminate();
  return 0;
}


/*
 * Called when there is data to be read on the pipe socket.
 */
int LlaOutThread::new_pipe_data() {
  dmx_data data;
  unsigned int data_read;
  int ret = m_pipe->Receive((uint8_t*) &data, sizeof(data), data_read);
  if (ret < 0)
  {
    qCritical() << "llaout: socket receive failed";
    return 0;
  }
  if (!m_client->SendDmx(data.universe, data.data,
                         data_read - sizeof(data.universe)))
    qWarning() << "llaout:: SendDmx() failed";
  return 0;
}


/*
 * Setup the LlaClient to communicate with the server.
 * @return true if the setup worked corectly.
 */
bool LlaOutThread::setup_client(lla::network::ConnectedSocket *socket) {
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
 * Cleanup after the main destructor has run
 */
void LlaStandaloneClient::cleanup() {
  if (m_tcp_socket)
  {
    if (m_ss)
      m_ss->RemoveSocket(m_tcp_socket);
    delete m_tcp_socket;
    m_tcp_socket = NULL;
  }

  if (m_ss)
    delete m_ss;
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
    m_ss = new lla::network::SelectServer();

  if (!m_tcp_socket)
  {
    m_tcp_socket = lla::network::TcpSocket::Connect("127.0.0.1",
                                                    LLA_DEFAULT_PORT);
    if (!m_tcp_socket)
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
void LlaEmbeddedServer::cleanup()
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
  options.http_port = lla::LlaServer::DEFAULT_HTTP_PORT;
  m_daemon = new lla::LlaDaemon(options);
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
    m_pipe_socket = new lla::network::PipeSocket();
    if (!m_pipe_socket->Init())
    {
      qWarning() << "llaout: pipe failed";
      delete m_pipe_socket;
      m_pipe_socket = NULL;
      delete m_daemon;
      m_daemon = NULL;
      return false;
    }
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
