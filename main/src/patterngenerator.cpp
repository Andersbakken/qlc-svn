/*
  Q Light Controller
  patterngenerator.cpp
  
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

#include <math.h>
#include <qframe.h>
#include <qptrlist.h>
#include <assert.h>
#include <qcombobox.h>
#include <qpointarray.h>

#include "common/types.h"
#include "common/deviceclass.h"
#include "common/logicalchannel.h"
#include "app.h"
#include "doc.h"
#include "patterngenerator.h"
#include "generatorarea.h"
#include "sequence.h"
#include "device.h"

extern App* _app;

PatternGenerator::PatternGenerator(QWidget* parent, const char* name)
  : UI_PatternGenerator(parent, name),

    m_width             ( 127 ),
    m_height            ( 127 ),
    m_xOffset           ( 127 ),
    m_yOffset           ( 127 ),
    m_density           (  .1 ),
    m_orientation       ( 0 ),
    m_omegax            ( 1 ),
    m_omegay            ( 1 ),
    m_phasex            ( .0174532 ),
    m_phasey            ( .0174532 ),
    m_area              ( new GeneratorArea(m_generatorFrame) ),
    m_pointArray        ( ),
    m_sequence          ( NULL ),
    m_horizontalChannel ( 0 ),
    m_verticalChannel   ( 0 )
{
  m_area->resize(m_generatorFrame->size());

}

PatternGenerator::~PatternGenerator()
{
}

void PatternGenerator::setSequence(Sequence* sequence)
{
  assert(sequence);
  m_sequence = sequence;

  Device* device = _app->doc()->device(sequence->device());
  assert(device);

  t_channel channels = device->deviceClass()->channels()->count();

  bool horizontal = false;
  bool vertical = false;

  QString s;
  for (t_channel ch = 0; ch < channels; ch++)
    {
      LogicalChannel* c = device->deviceClass()->channels()->at(ch);
      assert(c);
      
      // Insert ch:name strings to combos
      s.sprintf("%d:" + c->name(), ch);
      m_horizontalCombo->insertItem(s);
      m_verticalCombo->insertItem(s);

      // Select the first channel that contains the word "pan"
      if (horizontal == false && c->name().contains("pan", false))
	{
	  m_horizontalCombo->setCurrentItem(ch);
	  m_horizontalChannel = ch;
	  horizontal = true;
	}

      // Select the first channel that contains the word "tilt"
      if (vertical == false && c->name().contains("tilt", false))
	{
	  m_verticalCombo->setCurrentItem(ch);
	  m_verticalChannel = ch;
	  vertical = true; // Select the first that contains "tilt"
	}
    }

  createCircle();
}


void PatternGenerator::slotPatternSelected(const QString &text)
{
  if (m_patternCombo->currentText() == "Circle")
    {
      createCircle();
    }

  else if (m_patternCombo->currentText() == "Eight")
    {
      createEight();
    }
    
  else if (m_patternCombo->currentText() == "Line")
    {
      createLine();
    }
    
  else if (m_patternCombo->currentText() == "Square")
    {
      createSquare();
    }
    
  else if (m_patternCombo->currentText() == "Triangle")
    {
      createTriangle();
    }
    
  else if (m_patternCombo->currentText() == "Lissajou")
    {
      createLissajou();
    }
}

void PatternGenerator::slotWidthSpinChanged(int value)
{
  m_width = value;
  slotPatternSelected(m_patternCombo->currentText());
}

void PatternGenerator::slotHeightSpinChanged(int value)
{
  m_height = value;
  slotPatternSelected(m_patternCombo->currentText());
}

void PatternGenerator::slotXOffsetSpinChanged(int value)
{
  m_xOffset = value;
  slotPatternSelected(m_patternCombo->currentText());
}

void PatternGenerator::slotYOffsetSpinChanged(int value)
{
  m_yOffset = value;
  slotPatternSelected(m_patternCombo->currentText());
}

void PatternGenerator::slotDensitySpinChanged(int value)
{
  m_density = static_cast<double> (value) / 100.0;
  slotPatternSelected(m_patternCombo->currentText());
}

// New Function to
void PatternGenerator::slotOrientationChanged(int value)
{
  m_orientation = value;
  slotPatternSelected(m_patternCombo->currentText());
}

void PatternGenerator::slotOmegaXChanged(int value)
{
  m_omegax = value;
  slotPatternSelected(m_patternCombo->currentText());
}

void PatternGenerator::slotOmegaYChanged(int value)
{
  m_omegay = value;
  slotPatternSelected(m_patternCombo->currentText());
}

void PatternGenerator::slotPhaseXChanged(int value)
{
  m_phasex = static_cast<double> ((value) * M_PI) / 180.0;
  slotPatternSelected(m_patternCombo->currentText());
}

void PatternGenerator::slotPhaseYChanged(int value)
{
  m_phasey = static_cast<double> ((value) * M_PI) / 180.0;
  slotPatternSelected(m_patternCombo->currentText());
}

void PatternGenerator::slotHorizontalChannelSelected(int channel)
{
  m_horizontalChannel = channel;
}

void PatternGenerator::slotVerticalChannelSelected(int channel)
{
  m_verticalChannel = channel;
}

void PatternGenerator::createCircle()
{
  double i = 0;
  int x = 0;
  int y = 0;
  int point = 0;
  
  double pi = M_PI * 2;

  m_pointArray.resize(static_cast<int> (ceil(pi / m_density)));

  for (i = 0; i < pi; i += m_density)
    {
      x = static_cast<int> (m_xOffset + (cos(i + M_PI_2) * m_width));
      y = static_cast<int> (m_yOffset + (cos(i) * m_height));
      
      m_pointArray.setPoint(point++, x, y);
    }

  m_area->setPointArray(m_pointArray);
}

void PatternGenerator::createLine()
{
  double i = 0;
  int x = 0;
  int y = 0;
  int point = 0;
  
  double pi = M_PI * 2;

  m_pointArray.resize(static_cast<int> (ceil(pi / m_density)));

  for (i = 0; i < pi; i += m_density)
    {
      x = static_cast<int> (m_xOffset + (cos(i) * m_width));
      y = static_cast<int> (m_yOffset + (cos(i) * m_height));
      
      m_pointArray.setPoint(point++, x, y);
    }

  m_area->setPointArray(m_pointArray);
}

void PatternGenerator::createEight()
{
  double i = 0;
  int x = 0;
  int y = 0;

  int point = 0;

  double pi = M_PI * 2;

  m_pointArray.resize(static_cast<int> (ceil(pi / m_density)));

  for (i = 0; i < pi; i += m_density)
    {
      x = static_cast<int> (m_xOffset + (cos((i * 2) + M_PI_2) * m_width));
      y = static_cast<int> (m_yOffset + (cos(i) * m_height));
      
      m_pointArray.setPoint(point++, x, y);
    }
    
  m_area->setPointArray(m_pointArray);

}

void PatternGenerator::createSquare()
{
  double i = 0;
  int x = 0;
  int y = 0;
  int point = 0;
  
  double pi = M_PI * 2;

  m_pointArray.resize(static_cast<int> (ceil(pi / m_density)));

  for (i = 0; i < pi; i += m_density)
    {
      x = static_cast<int> (m_xOffset + (pow(cos(i - M_PI_2), 3) * m_width));
      y = static_cast<int> (m_yOffset + (pow(cos(i), 3) * m_height));
      
      m_pointArray.setPoint(point++, x, y);
    }

  m_area->setPointArray(m_pointArray);
}

void PatternGenerator::createTriangle()
{
  double i = 0;
  int x = 0;
  int y = 0;
  int point = 0;
  
  double pi = M_PI* 2;

  m_pointArray.resize(static_cast<int> (ceil(pi / m_density)));

  for (i = 0; i < pi; i += m_density)
    {
      x = static_cast<int> (m_xOffset + (cos(i) * m_width));
      y = static_cast<int> (m_yOffset + (sin(i) * m_height));
      
      m_pointArray.setPoint(point++, x, y);
    }

  m_area->setPointArray(m_pointArray);
}

void PatternGenerator::createLissajou()
{
  double i = 0;
  int x = 0;
  int y = 0;
  int point = 0;
  
  double pi = 3.14159 * 2;
  
  m_pointArray.resize(static_cast<int> (ceil(pi / m_density)));

  for (i = 0; i < pi; i += m_density)
    {
      x = static_cast<int> (m_xOffset + (cos((m_omegax * i) - m_phasex) * m_width));
      y = static_cast<int> (m_yOffset + (cos((m_omegay * i) - m_phasey) * m_height));
      
      m_pointArray.setPoint(point++, x, y);
    }

  m_area->setPointArray(m_pointArray);
}
