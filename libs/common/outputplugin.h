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

#include <qobject.h>

class OutputPlugin : public QObject
{
  Q_OBJECT

 public:
  OutputPlugin() {}
  virtual ~OutputPlugin() {}

  virtual bool open() = 0;
  virtual bool close() = 0;

  virtual void configure() = 0;

  virtual bool writeChannel(unsigned short channel, unsigned char value) = 0;
  virtual unsigned char readChannel(unsigned short channel) = 0;
};

#endif
