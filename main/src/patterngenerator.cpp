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

#include "app.h"
#include "doc.h"
#include "types.h"
#include "patterngenerator.h"
#include "generatorarea.h"
#include "sequence.h"
#include "device.h"
#include "deviceclass.h"
#include "logicalchannel.h"

extern App* _app;

PatternGenerator::PatternGenerator(QWidget* parent, const char* name)
  : UI_PatternGenerator(parent, name),

    m_width             ( 127 ),
    m_height            ( 127 ),
    m_xOffset           ( 127 ),
    m_yOffset           ( 127 ),
    m_density           (  .1 ),
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

void PatternGenerator::slotWidthSpinChanged(int value)
{
  m_width = value;
  createCircle();
}

void PatternGenerator::slotHeightSpinChanged(int value)
{
  m_height = value;
  createCircle();
}

void PatternGenerator::slotXOffsetSpinChanged(int value)
{
  m_xOffset = value;
  createCircle();
}

void PatternGenerator::slotYOffsetSpinChanged(int value)
{
  m_yOffset = value;
  createCircle();
}

void PatternGenerator::slotDensitySpinChanged(int value)
{
  m_density = static_cast<double> (value) / 100.0;
  createCircle();
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
  
  double pi = 3.14159 * 2;

  m_pointArray.resize(static_cast<int> (ceil(pi / m_density)));

  for (i = 0; i < pi; i += m_density)
    {
      x = static_cast<int> (m_yOffset + (sin(i) * m_width));
      y = static_cast<int> (m_xOffset + (cos(i) * m_height));
      
      m_pointArray.setPoint(point++, x, y);
    }

  m_area->setPointArray(m_pointArray);
}
