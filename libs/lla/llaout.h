/*
  Q Light Controller
  llaout.h
  
  Copyright (C) Simon Newton, Heikki Junnila
  
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

#include "../common/outputplugin.h"
#include "../../main/src/types.h"

#include <qptrlist.h>
#include <qstring.h>
#include <lla/LlaClient.h>

#define MAXINTERFACES 8

class ConfigureLlaOut;
class QPoint;

extern "C" OutputPlugin* create(t_plugin_id id);
extern "C" void destroy(OutputPlugin* object);

class LlaOut : public OutputPlugin
{
	Q_OBJECT
		
	friend class ConfigureLlaOut;
	
 public:
	LlaOut(t_plugin_id id);
	~LlaOut();
	
	int open();
	int close();
	bool isOpen();
	
	// Plugin methods
	int configure();
	QString infoText();
	void contextMenu(QPoint pos);
	
	
	int setConfigDirectory(QString dir);
	int saveSettings();
	int loadSettings();
	
	// OutputPlugin methods
	int writeChannel(t_channel channel, t_value value);
	int writeRange(t_channel address, t_value* values, t_channel num);
	
	int readChannel(t_channel channel, t_value &value);
	int readRange(t_channel address, t_value* values, t_channel num);
	
	int firstUni() { return m_firstUni; }
	
 private slots:
	void activate();
	
	void slotContextMenuCallback(int item);
	
 private:
	void createContents(QPtrList <QString> &list);
	
 private:
	QString m_configDir;
	LlaClient *m_lla;
	int m_firstUni;
	t_value m_values[KChannelMax];
};

#endif
