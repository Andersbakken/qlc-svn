/*
  Q Light Controller
  filehandler.cpp

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

#include "filehandler.h"
#include <qfile.h>

bool FileHandler::readFileToList(QString &fileName, QPtrList <QString> &list)
{
  QFile file(fileName);
  QString s = QString::null;
  QString t = QString::null;
  QString buf = QString::null;
  int i = 0;

  if (fileName == QString::null)
    {
      return false;
    }
  
  while (list.isEmpty() == false)
    {
      list.first();
      delete list.take();
    }

  if (file.open(IO_ReadOnly))
    {
      list.append(new QString("Entry"));
      list.append(new QString("Dummy"));

      // First read all entries to a string list
      while (file.atEnd() == false)
	{
	  file.readLine(buf, 1024);
	  
	  // If there is no "equal" sign on this row or it begins
	  // with a hash, ignore it
	  i = buf.find(QString("="));
	  if (i > -1 && buf.left(1) != QString("#"))
	    {
	      /* Get the string up to equal sign */
	      s = buf.mid(0, i).stripWhiteSpace();
	      list.append(new QString(s));

	      /* Get the string after the equal sign */
	      t = buf.mid(i + 1).stripWhiteSpace();
	      list.append(new QString(t));
	    }
	}

      file.close();
      return true;
    }
  else
    {
      return false;
    }
}
