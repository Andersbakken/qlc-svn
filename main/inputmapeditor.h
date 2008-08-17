/*
  Q Light Controller
  inputmaepditor.h
  
  Copyright (c) Heikki Junnila
  
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

#ifndef INPUTMAPEDITOR_H
#define INPUTMAPEDITOR_H

#include <QDialog>
#include "ui_inputmapeditor.h"

class InputMap;

class InputMapEditor : public QDialog, public Ui_InputMapEditor
{
	Q_OBJECT
public:
	InputMapEditor(QWidget* parent, InputMap* inputMap);
	~InputMapEditor();

	/*********************************************************************
	 * Slots
	 *********************************************************************/
protected slots:
	/**
	 * Slot for edit button clicks
	 */
	void slotEditClicked();

	/*********************************************************************
	 * OK & Cancel
	 *********************************************************************/
protected slots:
	/** Slot for OK button */
	void accept();

protected:
	/** The INPUTMap that we're editing here */
	InputMap* m_inputMap;
};

#endif
