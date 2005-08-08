/*
  Q Light Controller
  patterngenerator.h
  
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

#ifndef PATTERNGENERATOR_H
#define PATTERNGENERATOR_H

#include <qpointarray.h>

#include "uic_patterngenerator.h"
#include "types.h"

class QPaintEvent;
class GeneratorArea;
class Sequence;

class PatternGenerator : public UI_PatternGenerator
{
  Q_OBJECT

 public:
  PatternGenerator(QWidget* parent = NULL, const char* name = NULL);
  ~PatternGenerator();

  void setSequence(Sequence*);

  QPointArray pointArray() { return m_pointArray; }
  t_channel horizontalChannel() { return m_horizontalChannel; }
  t_channel verticalChannel() { return m_verticalChannel; }

 public slots:
  void slotPatternSelected (const QString &text);
  void slotWidthSpinChanged(int);
  void slotHeightSpinChanged(int);
  void slotXOffsetSpinChanged(int);
  void slotYOffsetSpinChanged(int);
  void slotDensitySpinChanged(int);
  void slotOrientationChanged(int);
  
  void slotHorizontalChannelSelected(int);
  void slotVerticalChannelSelected(int);
  

 protected:
  void createCircle();
  void createLine();
  void createEight();
  void createSquare();
  void createTriangle();
 
 protected:
  double m_width;
  double m_height;
  double m_xOffset;
  double m_yOffset;
  double m_density;
  double m_orientation;

 protected:
  GeneratorArea* m_area;
  QPointArray m_pointArray;

  Sequence* m_sequence;

  t_channel m_horizontalChannel;
  t_channel m_verticalChannel;
};

#endif
