/*
  Q Light Controller
  efxeditor.cpp
  
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

#include "efxeditor.h"
#include "app.h"
#include "doc.h"
#include "device.h"
#include "logicalchannel.h"
#include "deviceclass.h"

#include <qpen.h>
#include <qevent.h>
#include <qpainter.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <assert.h>

extern App* _app;

EFXEditor::EFXEditor(EFX* efx, QWidget* parent)
  : UI_EFXEditor(parent, "EFXEditor", true),

    m_previewArea ( new EFXPreviewArea(m_previewFrame) ),
    m_efx         ( efx )
{
}

EFXEditor::~EFXEditor()
{
  m_efx->setPreviewPointArray(NULL);
}

void EFXEditor::init()
{
  m_nameEdit->setText(m_efx->name());

  m_previewArea->resize(m_previewFrame->width(),
			m_previewFrame->height());

  setEFX(m_efx);
}

void EFXEditor::setEFX(EFX* efx)
{
  assert(efx);
  m_efx = efx;

  /* Get supported algorithms and fill the algorithm combo with them */
  QStringList list;
  EFX::algorithmList(list);
  m_algorithmCombo->clear();
  m_algorithmCombo->insertStringList(list);

  m_efx->setPreviewPointArray(m_previewArea->pointArray());

  /* Select the EFX's algorithm from the algorithm combo */
  for (int i = 0; i < m_algorithmCombo->count(); i++)
    {
      if (m_algorithmCombo->text(i) == m_efx->algorithm())
	{
	  m_algorithmCombo->setCurrentItem(i);
	  break;
	}
    }

  fillChannelCombos();
}

void EFXEditor::fillChannelCombos()
{
  assert(m_efx);

  Device* device = _app->doc()->device(m_efx->device());
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
}

void EFXEditor::slotNameChanged(const QString &text)
{
  assert(m_efx);

  setCaption(QString("EFX Editor - ") + text);

  m_efx->setName(text);
}

void EFXEditor::slotAlgorithmSelected(const QString &text)
{
  assert(m_efx);

  m_efx->setAlgorithm(text);
}

void EFXEditor::slotWidthSpinChanged(int value)
{
  assert(m_efx);

  m_efx->setWidth(value);
}

void EFXEditor::slotHeightSpinChanged(int value)
{
  assert(m_efx);

  m_efx->setHeight(value);
}

void EFXEditor::slotXOffsetSpinChanged(int value)
{
  assert(m_efx);

  m_efx->setXOffset(value);
}

void EFXEditor::slotYOffsetSpinChanged(int value)
{
  assert(m_efx);

  m_efx->setYOffset(value);
}

void EFXEditor::slotHorizontalChannelSelected(int channel)
{
  assert(m_efx);

  m_efx->setXChannel(static_cast<t_channel> (channel));
}

void EFXEditor::slotVerticalChannelSelected(int channel)
{
  assert(m_efx);

  m_efx->setYChannel(static_cast<t_channel> (channel));
}


/*****************************************************************************
 * EFX Preview Area implementation
 *****************************************************************************/

/**
 * Constructor
 */
EFXPreviewArea::EFXPreviewArea(QWidget* parent, const char* name)
  : QFrame (parent, name),
    
    m_pointArray ( new QPointArray )
{
  setPaletteBackgroundColor(white);
  setFrameStyle(StyledPanel | Sunken);
}

/**
 * Destructor
 */
EFXPreviewArea::~EFXPreviewArea()
{
  setUpdatesEnabled(false);

  delete m_pointArray;
  m_pointArray = NULL;
}

/**
 * Get the pointer for the point array that is used
 * to draw the preview
 *
 * @return The point array
 */
QPointArray* EFXPreviewArea::pointArray()
{
  return m_pointArray;
}

/**
 * Paint the points in the point array
 */
void EFXPreviewArea::paintEvent(QPaintEvent* e)
{
  QFrame::paintEvent(e);

  QPainter painter(this);
  QPen pen;
  QPoint point;

  // Draw crosshairs
  painter.setPen(lightGray);
  painter.drawLine(127, 0, 127, 255);
  painter.drawLine(0, 127, 255, 127);

  // Set pen color to black
  pen.setColor(black);

  // Use the black pen as the painter
  painter.setPen(pen);

  // Draw the points from the point array
  for (unsigned int i = 0; isUpdatesEnabled() && i < m_pointArray->size(); i++)
    {
      point = m_pointArray->point(i);
      painter.drawPoint(point);

      // Draw a small ellipse around each point
      painter.drawEllipse(point.x() - 3, point.y() - 3, 6, 6);
    }
}
