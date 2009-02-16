/*
  Q Light Controller
  llaoutthread.h

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

#ifndef LLAOUTTHREAD_H
#define LLAOUTTHREAD_H

#include <qthread.h>
#include <lla/LlaClient.h>
#include <lla/select_server/SelectServer.h>
#include <lla/select_server/Socket.h>
#include <llad/LlaDaemon.h>

// This should really be in qlctypes.h!
enum { K_UNIVERSE_SIZE = 512 };

// Used to pass data between the threads
typedef struct
{
  unsigned int universe;
  dmx_t data[K_UNIVERSE_SIZE];
} dmx_data;


// Used in the LLA thread to handle data send through the pipe
class PipeListener : public lla::select_server::SocketListener
{
public:
  PipeListener(lla::LlaClient *client): m_client(client) {}
  int SocketReady(lla::select_server::ConnectedSocket *socket);

private:
  lla::LlaClient *m_client;
  dmx_data m_data;
};


/*
 * The LLA thread.
 *
 * Basic design: qlc plugins aren't allowed to block in calls, so we start a
 * new thread which runs a select server. Calls to write_dmx in the plugin send
 * data over a pipe which the LLA thread listens on. It then uses the LlaClient
 * api to send the data to the LLA Server.
 *
 * The thread can either run as a LLA Client or embed the LLA server. As a
 * client, we connect to the LLA server using a TCP socket.
 *
 *   LlaOut --pipe-> LlaOutThread --tcp socket-> llad (separate process)
 *
 * When embedded the server, we still use the LlaClient class and setup a pipe
 * to send the * rpcs over. Yes, this results in copying the data twice over a
 * pipe but we * can't use a single pipe because the LlaClient needs to
 * response to events.
 *
 * LlaOut --pipe-> LlaOutThread --pipe-> LlaServer
 */
class LlaOutThread : public QThread
{
public:
  /*
   * @param run_as_embedded Set to true to run the embedded LLA server
   */
  LlaOutThread():
    m_init_run(false),
    m_ss(NULL),
    m_pipe(NULL),
    m_listener(NULL),
    m_client(NULL) {}
  virtual ~LlaOutThread();

  void run();
  bool start(Priority priority=InheritPriority);
  void stop();
  int write_dmx(unsigned int universe, dmx_t *data, unsigned int channels);

protected:
  bool setup_client(lla::select_server::ConnectedSocket *socket);
  bool m_init_run;
  lla::select_server::SelectServer *m_ss; // the select server

private:
  virtual bool init() = 0;
  lla::select_server::LoopbackSocket *m_pipe; // the pipe to get new dmx data on
  lla::select_server::SocketListener *m_listener; // the listener for the pipe
  lla::LlaClient *m_client;
  dmx_data m_data;
};


/*
 * Use this to run as a standalone client.
 */
class LlaStandaloneClient : public LlaOutThread
{
public:
  LlaStandaloneClient():
    LlaOutThread(),
    m_tcp_socket(NULL) {}
  ~LlaStandaloneClient();

private:
  bool init();
  lla::select_server::TcpSocket *m_tcp_socket;
};


/*
 * Use this to run with an embedded server.
 */
class LlaEmbeddedServer : public LlaOutThread
{
public:
  LlaEmbeddedServer():
    LlaOutThread(),
    m_daemon(NULL),
    m_pipe_socket(NULL) {}
  ~LlaEmbeddedServer();

private:
  bool init();
  lla::LlaDaemon *m_daemon;
  lla::select_server::PipeSocket *m_pipe_socket;
};

#endif
