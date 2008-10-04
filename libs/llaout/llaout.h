/*
  Q Light Controller
  llaout.h
  
  Copyright (c) Simon Newton
                Heikki Junnila
  
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

#ifndef LLAOUT_H
#define LLAOUT_H

#include <QObject>
#include <QMutex>
#include <QDebug>

#include "common/qlcoutplugin.h"
#include "common/qlctypes.h"

class LlaClient
#ifndef WIN32
;
#else
{
public:
	LlaClient() {}
	~LlaClient() {}
	
	int start() { qDebug() << "LLA Start"; return 0; }
	void stop() { qDebug() << "LLA Stop"; }
	void send_dmx(unsigned int u, t_value* v, unsigned int n)
	{
		qDebug() << "LLA Send: " << u << v << n;
	}
	void fd_action(unsigned int a) { qDebug() << "LLA FD: " << a; }
};
#endif

class ConfigureLlaOut;

class LLAOut : public QObject, public QLCOutPlugin
{
	Q_OBJECT
	Q_INTERFACES(QLCOutPlugin)

	friend class ConfigureLlaOut;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	void init();
	
	/*********************************************************************
	 * Open/close
	 *********************************************************************/
public:
	void open(t_output output = 0);
	void close(t_output output = 0);
	QStringList outputs();

protected:
	LlaClient *m_lla;
	
	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	QString name();
	
	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	void configure();

protected:
	QString m_configDir;

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	QString infoText(t_output output = KOutputInvalid);
	
	/*********************************************************************
	 * Value read/write
	 *********************************************************************/
public:
	void writeChannel(t_output output, t_channel channel, t_value value);
	void writeRange(t_output output, t_channel address, t_value* values,
			t_channel num);

	void readChannel(t_output output, t_channel channel, t_value* value);
	void readRange(t_output output, t_channel address, t_value* values,
		       t_channel num);

protected:
	t_value m_values[512];
	QMutex m_mutex;
};

#endif
