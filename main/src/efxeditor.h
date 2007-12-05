/*
  Q Light Controller
  efxeditor.h
  
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

#ifndef EFXEDITOR_H
#define EFXEDITOR_H

#include <qpointarray.h>
#include <qframe.h>

#include "common/types.h"
#include "uic_efxeditor.h"
#include "efx.h"

class QPaintEvent;
class EFXPreviewArea;

class EFXEditor : public UI_EFXEditor
{
	Q_OBJECT

public:
	EFXEditor(QWidget* parent, EFX* efx);
	~EFXEditor();

protected:
	/**
	 * Set the EFX function to edit
	 *
	 * @param efx The EFX function to edit
	 */
	void setEFX(EFX* efx);

	/**
	 * Get channels from the EFX function's fixture
	 * and fill the combos with them.
	 */
	void fillChannelCombos();

	/**
	 * Get sceness from the EFX function's fixture
	 * and fill the list views with them.
	 */
	void fillSceneLists();

	/**
	 * Update the list of available buses
	 *
	 */
	void updateModulationBusCombo();

protected slots:
	void slotNameChanged(const QString &text);

	void slotAlgorithmSelected(const QString &text);
	void slotWidthSpinChanged(int value);
	void slotHeightSpinChanged(int value);
	void slotXOffsetSpinChanged(int value);
	void slotYOffsetSpinChanged(int value);
	void slotRotationSpinChanged(int value);

	void slotXFrequencySpinChanged(int value);
	void slotYFrequencySpinChanged(int value);
	void slotXPhaseSpinChanged(int value);
	void slotYPhaseSpinChanged(int value);

	void slotHorizontalChannelSelected(int index);
	void slotVerticalChannelSelected(int index);
  
	void slotStartSceneCheckboxToggled(bool);
	void slotStopSceneCheckboxToggled(bool);

	void slotStartSceneListSelectionChanged(QListViewItem* item);
	void slotStopSceneListSelectionChanged(QListViewItem* item);

	void slotDirectionClicked(int item);
	void slotRunOrderClicked(int item);

protected:
	EFXPreviewArea* m_previewArea;
	QPointArray* m_pointArray;

	EFX* m_efx;
};

/**
 * The area that is used to draw a preview of
 * the EFX function currently being edited.
 */
class EFXPreviewArea : public QFrame
{
	Q_OBJECT

public:
	EFXPreviewArea(QWidget* parent);
	~EFXPreviewArea();

	/**
	 * Get the pointer for the point array that is used
	 * to draw the preview
	 *
	 * @return The point array
	 */
	QPointArray* pointArray();

protected:
	/**
	 * QT Framework calls this when the widget needs
	 * to be repainted.
	 *
	 * @param e QPaintEvent
	 */
	void paintEvent(QPaintEvent* e);

	QPointArray* m_pointArray;
};


#endif
