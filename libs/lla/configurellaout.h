/*
  Q Light Controller
  configurellaout.h
  
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

#ifndef CONFIGURELLAOUT_H
#define CONFIGURELLAOUT_H

#include "uic_configurellaout.h"

class LlaOut;

class ConfigureLlaOut : public UI_ConfigureLlaOut
{
	Q_OBJECT
		
 public:
	ConfigureLlaOut(LlaOut* plugin);
	virtual ~ConfigureLlaOut();
	
	void updateStatus();
	int firstDeviceID();
 
 private slots:
	void slotActivateClicked();
	
 private:
	LlaOut* m_plugin;
};

#endif
