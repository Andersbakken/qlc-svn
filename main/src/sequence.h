/*
  Q Light Controller
  sequence.h
  
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

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "pattern.h"
#include <qstring.h>
#include <qlist.h>

// Number of channels per sequence (or number of patterns per sequence)
#define SEQUENCE_SIZE	64

class Sequence 
{
 public: 
  Sequence(bool create_nodes = true);
  ~Sequence();
  
  QString m_description;
  int m_speed;
  
  QList <Pattern> m_patternList;
};

#endif
