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

#include <QPaintEvent>
#include <QPalette>
#include <QPainter>
#include <QPen>

#include "generatorarea.h"

GeneratorArea::GeneratorArea(QWidget* parent) : QFrame(parent)
{
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::white);
    setPalette(p);

    setFrameStyle(StyledPanel | Sunken);
}

GeneratorArea::~GeneratorArea()
{
}

void GeneratorArea::setPoints(const QPolygon& points)
{
    m_points = points;
    repaint();
}


void GeneratorArea::paintEvent(QPaintEvent* e)
{
    QFrame::paintEvent(e);

    QPainter painter(this);
    QPoint point;
    QPen pen;

    // Draw crosshairs
    painter.setPen(Qt::lightGray);
    painter.drawLine(127, 0, 127, 255);
    painter.drawLine(0, 127, 255, 127);

    // Draw points
    pen.setColor(Qt::black);
    painter.setPen(pen);

    for (int i = 0; i < m_points.size(); i++)
    {
        point = m_points.point(i);
        painter.drawPoint(point);
        painter.drawEllipse(point.x() - 3, point.y() - 3, 6, 6);
    }
}
