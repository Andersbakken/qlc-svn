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

#include "common/outputplugin.h"
#include "common/types.h"

class ConfigureLlaOut;
class LlaClient;

extern "C" OutputPlugin* create();

class LlaOut : public OutputPlugin
{
	Q_OBJECT
		
	friend class ConfigureLlaOut;
	
	/*********************************************************************
	 * Initialization
	 *********************************************************************/
 public:
	LlaOut();
	~LlaOut();
	
	/*********************************************************************
	 * Open/close
	 *********************************************************************/
public:
	int open();
	int close();
	int outputs();

protected:
	LlaClient *m_lla;
	
	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	int configure(QWidget* parentWidget);

protected:
	QString m_configDir;

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	QString infoText();
	
	/*********************************************************************
	 * Value read/write
	 *********************************************************************/
public:
	int writeChannel(t_channel channel, t_value value);
	int writeRange(t_channel address, t_value* values, t_channel num);
	
	int readChannel(t_channel channel, t_value &value);
	int readRange(t_channel address, t_value* values, t_channel num);
	
protected:
	t_value m_values[KChannelMax];
};

#endif
