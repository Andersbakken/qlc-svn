/*
  Q Light Controller
  olaoutthread.cpp

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
#include <ola/Closure.h>
#include "olaoutthread.h"


/*
 * Clean up.
 */
OlaOutThread::~OlaOutThread()
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
 * Start the OLA thread
 *
 * @return true if sucessfull, false otherwise
 */
bool OlaOutThread::start(Priority priority)
{
    if (!init())
        return false;

    if (!m_pipe)
    {
        // setup the pipe to recv dmx data on
        m_pipe = new ola::network::LoopbackSocket();
        m_pipe->Init();

        m_pipe->SetOnData(ola::NewClosure(this, &OlaOutThread::new_pipe_data));
        m_pipe->SetOnClose(ola::NewSingleClosure(this, &OlaOutThread::pipe_closed));
        m_ss->AddSocket(m_pipe);
    }

    QThread::start(priority);
    return true;
}


/*
 * Close the socket which stops the thread.
 */
void OlaOutThread::stop()
{
    if (m_pipe)
        m_pipe->CloseClient();
    return;
}


/*
 * Run the select server.
 */
void OlaOutThread::run()
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
int OlaOutThread::write_dmx(unsigned int universe, const QByteArray& data)
{
    m_data.universe = universe;
    memcpy(m_data.data, data.data(), data.size());
    if (m_pipe)
        m_pipe->Send((uint8_t*) &m_data, sizeof(m_data));
    return 0;
}


/*
 * Called when the pipe used to communicate between QLC and OLA is closed
 */
void OlaOutThread::pipe_closed() {
    // We don't need to delete the socket here because that gets done in the
    // Destructor.
    m_ss->Terminate();
}


/*
 * Called when there is data to be read on the pipe socket.
 */
void OlaOutThread::new_pipe_data() {
    dmx_data data;
    unsigned int data_read;
    int ret = m_pipe->Receive((uint8_t*) &data, sizeof(data), data_read);
    if (ret < 0)
    {
        qCritical() << "olaout: socket receive failed";
        return;
    }
    m_buffer.Set(data.data, data_read - sizeof(data.universe));
    if (!m_client->SendDmx(data.universe, m_buffer))
        qWarning() << "olaout:: SendDmx() failed";
}


/*
 * Setup the OlaClient to communicate with the server.
 * @return true if the setup worked corectly.
 */
bool OlaOutThread::setup_client(ola::network::ConnectedSocket *socket) {
    if (!m_client)
    {
        m_client = new ola::OlaClient(socket);
        if (!m_client->Setup())
        {
            qWarning() << "olaout: client setup failed";
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
void OlaStandaloneClient::cleanup() {
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
bool OlaStandaloneClient::init()
{
    if (m_init_run)
        return true;

    if (!m_ss)
        m_ss = new ola::network::SelectServer();

    if (!m_tcp_socket)
    {
        m_tcp_socket = ola::network::TcpSocket::Connect("127.0.0.1",
                       OLA_DEFAULT_PORT);
        if (!m_tcp_socket)
        {
            qWarning() << "olaout: Connect failed, is OLAD running?";
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
void OlaEmbeddedServer::cleanup()
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
bool OlaEmbeddedServer::init()
{
    if (m_init_run)
        return true;

    ola::ola_server_options options;
    options.http_enable = true;
    options.http_port = ola::OlaServer::DEFAULT_HTTP_PORT;
    m_daemon = new ola::OlaDaemon(options);
    if (!m_daemon->Init())
    {
        qWarning() << "OLA Server failed init";
        delete m_daemon;
        m_daemon = NULL;
        return false;
    }
    m_ss = m_daemon->GetSelectServer();

    // setup the pipe socket used to communicate with the OlaServer
    if (!m_pipe_socket)
    {
        m_pipe_socket = new ola::network::PipeSocket();
        if (!m_pipe_socket->Init())
        {
            qWarning() << "olaout: pipe failed";
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

    m_daemon->GetOlaServer()->NewConnection(m_pipe_socket->OppositeEnd());
    m_init_run = true;
    return true;
}
