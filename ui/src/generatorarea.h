/*
  Q Light Controller
  generatorarea.h

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

#ifndef GENERATORAREA_H
#define GENERATORAREA_H

#include <QPolygon>
#include <QFrame>

class QPaintEvent;

class GeneratorArea : public QFrame
{
    Q_OBJECT

public:
    GeneratorArea(QWidget* parent);
    ~GeneratorArea();

    /** Set the points to draw */
    void setPoints(const QPolygon& points);

private:
    Q_DISABLE_COPY(GeneratorArea)

protected:
    void paintEvent(QPaintEvent* event);

    /** The array of points to draw */
    QPolygon m_points;
};

#endif
