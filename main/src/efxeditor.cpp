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
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qlabel.h>
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
  /* Get supported algorithms and fill the algorithm combo with them */
  QStringList list;
  EFX::algorithmList(list);
  m_algorithmCombo->clear();
  m_algorithmCombo->insertStringList(list);

  /* Get a list of buses and insert them into the bus combo */
  updateModulationBusCombo();

  /* Set the algorithm's name to the name field */
  m_nameEdit->setText(m_efx->name());

  /* Resize the preview area to fill its frame */
  m_previewArea->resize(m_previewFrame->width(),
			m_previewFrame->height());

  /* Set the currently edited EFX function */
  setEFX(m_efx);

  /* Draw the points */
  m_previewArea->repaint();
}

void EFXEditor::setEFX(EFX* efx)
{
  assert(efx);

  if (m_efx)
    {
      /* If another EFX function has been edited with this dialog,
       * set its preview point array NULL so it doesn't try to
       * update its preview anymore
       */
      m_efx->setPreviewPointArray(NULL);
    }

  /* Causes the EFX function to update the preview point array */
  slotAlgorithmSelected(m_efx->algorithm());

  /* Take the new EFX function for editing */
  m_efx = efx;

  /* Set the preview point array for the new EFX */
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

  /* Get the algorithm parameters */
  m_widthSpin->setValue(m_efx->width());
  m_heightSpin->setValue(m_efx->height());
  m_xOffsetSpin->setValue(m_efx->xOffset());
  m_yOffsetSpin->setValue(m_efx->yOffset());
  m_rotationSpin->setValue(m_efx->rotation());
  
  m_xFrequencySpin->setValue(m_efx->xFrequency());
  m_yFrequencySpin->setValue(m_efx->yFrequency());
  m_xPhaseSpin->setValue(m_efx->xPhase());
  m_yPhaseSpin->setValue(m_efx->yPhase());

  /* Get advanced parameters */
  m_runOrderGroup->setButton(m_efx->runOrder());
  m_directionGroup->setButton(m_efx->direction());

  m_modulationBusCombo->setCurrentItem(m_efx->modulationBus());

  fillChannelCombos();
  fillSceneCombos();
}

void EFXEditor::fillChannelCombos()
{
  assert(m_efx);

  Device* device = _app->doc()->device(m_efx->device());
  assert(device);

  t_channel channels = device->deviceClass()->channels()->count();

  QString s;
  for (t_channel ch = 0; ch < channels; ch++)
    {
      LogicalChannel* c = device->deviceClass()->channels()->at(ch);
      assert(c);
      
      // Insert ch:name strings to combos
      s.sprintf("%d:" + c->name(), ch + 1); /* Display channels as 1-based */
      m_horizontalCombo->insertItem(s);
      m_verticalCombo->insertItem(s);
    }

  /* Select a channel as the X axis */
  if (m_efx->xChannel() != KChannelInvalid)
    {
      /* If the EFX already has a valid x channel, select it instead */
      m_horizontalCombo->setCurrentItem(m_efx->xChannel());
    }
  else
    {
      for (t_channel ch = 0; ch < channels; ch++)
	{
	  LogicalChannel* c = device->deviceClass()->channels()->at(ch);
	  assert(c);
      
	  // Select the first channel that contains the word "pan"
	  if (c->name().contains("pan", false))
	    {
	      m_horizontalCombo->setCurrentItem(ch);
	      m_efx->setXChannel(ch);
	      break;
	    }
	}
    }

  /* Select a channel as the X axis */
  if (m_efx->yChannel() != KChannelInvalid)
    {
      /* If the EFX already has a valid y channel, select it instead */
      m_verticalCombo->setCurrentItem(m_efx->yChannel());
    }
  else
    {  
      for (t_channel ch = 0; ch < channels; ch++)
	{
	  LogicalChannel* c = device->deviceClass()->channels()->at(ch);
	  assert(c);
      
	  // Select the first channel that contains the word "tilt"
	  if (c->name().contains("tilt", false))
	    {
	      m_verticalCombo->setCurrentItem(ch);
	      m_efx->setYChannel(ch);
	      break;
	    }
	}
    }
}

void EFXEditor::fillSceneCombos()
{
  assert(m_efx);
  
//  Device* device = _app->doc()->device(m_efx->device());
//  assert(device);
  
  QString s;
  for (t_function_id id = 0; id < KFunctionArraySize; id++)
    {
      Function* f = _app->doc()->function(id);
      if (!f)
        continue;
	
      if (f->type() == Function::Scene && f->device() == m_efx->device())
        {
      
      // Insert id:name strings to combos
          s.sprintf("%d:" + f->name(), id);
          m_startSceneCombo->insertItem(s);
          m_stopSceneCombo->insertItem(s);
        }
    }
    
  if (m_efx->startScene() > KNoID)
    {
      m_startSceneCheckbox->setChecked(true);
      /* If the EFX already has a valid start function, select it instead */
      for (int i = 0; i < m_startSceneCombo->count(); i++)
	{
	  QString cvs = m_startSceneCombo->text(i);
          if (cvs.section(':', 0, 0).toInt() == m_efx->startScene())
	    {
	      m_startSceneCombo->setCurrentItem(i);
	      break;
	    }
	}
    }
  else
    {
      m_startSceneCheckbox->setChecked(false);
      m_startSceneCombo->setEnabled(false);
    }

  if (m_efx->stopScene() > KNoID)
    {
      m_stopSceneCheckbox->setChecked(true);
      /* If the EFX already has a valid stop function, select it instead */
      for (int i = 0; i < m_stopSceneCombo->count(); i++)
	{
	  QString cvs = m_stopSceneCombo->text(i);
          if (cvs.section(':', 0, 0).toInt() == m_efx->stopScene())
	    {
	      m_stopSceneCombo->setCurrentItem(i);
	      break;
	    }
	}
    }
  else
    {
      m_stopSceneCheckbox->setChecked(false);
      m_stopSceneCombo->setEnabled(false);
    }
}

void EFXEditor::updateModulationBusCombo()
{
  m_modulationBusCombo->clear();

  for (t_bus_id i = KBusIDMin; i < KBusCount; i++)
    {
      QString bus;
      bus.sprintf("%.2d:", i + 1);
      bus += Bus::name(i);
      m_modulationBusCombo->insertItem(bus, i);
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

  if (m_efx->isFrequencyEnabled())
    {
      m_xFrequencyLabel->setEnabled(true);
      m_yFrequencyLabel->setEnabled(true);

      m_xFrequencySpin->setEnabled(true);
      m_yFrequencySpin->setEnabled(true);
    }
  else
    {
      m_xFrequencyLabel->setEnabled(false);
      m_yFrequencyLabel->setEnabled(false);

      m_xFrequencySpin->setEnabled(false);
      m_yFrequencySpin->setEnabled(false);
    }

  if (m_efx->isPhaseEnabled())
    {
      m_xPhaseLabel->setEnabled(true);
      m_yPhaseLabel->setEnabled(true);

      m_xPhaseSpin->setEnabled(true);
      m_yPhaseSpin->setEnabled(true);
    }
  else
    {
      m_xPhaseLabel->setEnabled(false);
      m_yPhaseLabel->setEnabled(false);

      m_xPhaseSpin->setEnabled(false);
      m_yPhaseSpin->setEnabled(false);
    }


  m_previewArea->repaint();
}

void EFXEditor::slotWidthSpinChanged(int value)
{
  assert(m_efx);

  m_efx->setWidth(value);

  m_previewArea->repaint();
}

void EFXEditor::slotHeightSpinChanged(int value)
{
  assert(m_efx);

  m_efx->setHeight(value);

  m_previewArea->repaint();
}

void EFXEditor::slotRotationSpinChanged(int value)
{
  assert(m_efx);

  m_efx->setRotation(value);

  m_previewArea->repaint();
}

void EFXEditor::slotXOffsetSpinChanged(int value)
{
  assert(m_efx);

  m_efx->setXOffset(value);

  m_previewArea->repaint();
}

void EFXEditor::slotYOffsetSpinChanged(int value)
{
  assert(m_efx);

  m_efx->setYOffset(value);

  m_previewArea->repaint();
}

void EFXEditor::slotXFrequencySpinChanged(int value)
{
  assert(m_efx);

  m_efx->setXFrequency(value);

  m_previewArea->repaint();
}

void EFXEditor::slotYFrequencySpinChanged(int value)
{
  assert(m_efx);

  m_efx->setYFrequency(value);

  m_previewArea->repaint();
}

void EFXEditor::slotXPhaseSpinChanged(int value)
{
  assert(m_efx);

  m_efx->setXPhase(value);

  m_previewArea->repaint();
}

void EFXEditor::slotYPhaseSpinChanged(int value)
{
  assert(m_efx);

  m_efx->setYPhase(value);

  m_previewArea->repaint();
}

void EFXEditor::slotHorizontalChannelSelected(int channel)
{
  assert(m_efx);

  m_efx->setXChannel(static_cast<t_channel> (channel));

  m_previewArea->repaint();
}

void EFXEditor::slotVerticalChannelSelected(int channel)
{
  assert(m_efx);

  m_efx->setYChannel(static_cast<t_channel> (channel));

  m_previewArea->repaint();
}

void EFXEditor::slotStartSceneCheckboxToggled(bool state)
{
  assert(m_efx);

  if (state)
    {
      m_startSceneCombo->setEnabled(true);
    }
  else
    {
      m_startSceneCombo->setEnabled(false);
      t_function_id scene = KNoID;
      m_efx->setStartScene(static_cast<t_function_id> (scene));
    }
}


void EFXEditor::slotStopSceneCheckboxToggled(bool state)
{
  assert(m_efx);

  if (state == true)
    {
      m_stopSceneCombo->setEnabled(true);
    }
  else
    {
      m_stopSceneCombo->setEnabled(false);
      t_function_id scene = KNoID;
      m_efx->setStopScene(static_cast<t_function_id> (scene));
    }
}

void EFXEditor::slotStartSceneComboSelected(int scene)
{
  assert(m_efx);

  QString csv = m_startSceneCombo->currentText();
  QString s = csv.section(':', 0, 0);
  m_efx->setStartScene(s.toInt());
}

void EFXEditor::slotStopSceneComboSelected(int scene)
{
  assert(m_efx);
  
  QString csv = m_stopSceneCombo->currentText();
  QString s = csv.section(':', 0, 0);
  m_efx->setStopScene(s.toInt());
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
  //QPoint prevPoint;

  // Draw crosshairs
  painter.setPen(lightGray);
  painter.drawLine(127, 0, 127, 255);
  painter.drawLine(0, 127, 255, 127);

  // Set pen color to black
  pen.setColor(black);

  // Use the black pen as the painter
  painter.setPen(pen);

  painter.drawPolygon(*m_pointArray);

  // Take the last point so that the first line is drawn
  // from the last to the first
  // prevPoint = m_pointArray->point(m_pointArray->size() - 1);

  // Draw the points from the point array
  for (unsigned int i = 0; isUpdatesEnabled() && i < m_pointArray->size(); i++)
    {
      point = m_pointArray->point(i);
      //painter.drawPoint(point);
      //painter.drawLine(prevPoint, point);

      // Draw a small ellipse around each point
      painter.drawEllipse(point.x() - 2, point.y() - 2, 4, 4);

      //prevPoint = point;
    }
}
