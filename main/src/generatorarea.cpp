/*
  Q Light Controller
  generatorarea.cpp
  
  Copyright (C) Heikki Junnila
  
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

#include <qpen.h>
#include <qevent.h>
#include <qpainter.h>
#include "generatorarea.h"

GeneratorArea::GeneratorArea(QWidget* parent, const char* name)
  : QFrame (parent, name)
{
  setPaletteBackgroundColor(white);
  setFrameStyle(StyledPanel | Sunken);
}

GeneratorArea::~GeneratorArea()
{
}

void GeneratorArea::setPointArray(QPointArray& array)
{
  m_pointArray = array;
  repaint();
}


void GeneratorArea::paintEvent(QPaintEvent* e)
{
  QFrame::paintEvent(e);

  QPainter painter(this);
  QPen pen;

  // Draw crosshairs
  painter.setPen(lightGray);
  painter.drawLine(127, 0, 127, 255);
  painter.drawLine(0, 127, 255, 127);

  // Draw points
  pen.setColor(black);
  painter.setPen(pen);

  QPoint point;
  for (int i = 0; i < m_pointArray.size(); i++)
    {
      point = m_pointArray.point(i);
      painter.drawPoint(point);

      painter.drawEllipse(point.x() - 3, point.y() - 3, 6, 6);
    }
}
