/*
  Q Light Controller
  outputplugin.h

  Copyright (C) 2000, 2001, 2002 Heikki Junnila

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

#ifndef OUTPUTPLUGIN_H
#define OUTPUTPLUGIN_H

#include "plugin.h"

class OutputPlugin : public Plugin
{
  Q_OBJECT

 public:
  OutputPlugin(int id);
  virtual ~OutputPlugin();

  virtual bool writeChannel(unsigned short channel, unsigned char value) = 0;
  virtual bool writeRange(unsigned short address, unsigned char* values,
			  unsigned short num) = 0;

  virtual bool readChannel(unsigned short channel, unsigned char &value) = 0;
  virtual bool readRange(unsigned short address, unsigned char* values,
			 unsigned short num) = 0;
};

#endif
