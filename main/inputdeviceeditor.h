/*
  Q Light Controller
  inputdeviceeditor.h

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

#ifndef INPUTDEVICEEDITOR_H
#define INPUTDEVICEEDITOR_H

#include <common/qlctypes.h>
#include <QDialog>

#include "ui_inputdeviceeditor.h"

class QLCInputChannel;
class QLCInputDevice;

class InputDeviceEditor : public QDialog, public Ui_InputDeviceEditor
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	InputDeviceEditor(QWidget* parent, QLCInputDevice* device);
	virtual ~InputDeviceEditor();

private:
	Q_DISABLE_COPY(InputDeviceEditor)

protected:
	void fillTree();
	void updateChannelItem(QTreeWidgetItem* item, QLCInputChannel* ch);

	/********************************************************************
	 * OK & Cancel
	 ********************************************************************/
public slots:
	void reject();
	void accept();

	/********************************************************************
	 * Editing
	 ********************************************************************/
protected slots:
	void slotAddClicked();
	void slotRemoveClicked();
	void slotEditClicked();
	void slotWizardClicked(bool checked);

	void slotInputValueChanged(t_input_universe universe,
				   t_input_channel channel,
				   t_input_value value);

	/********************************************************************
	 * Device
	 ********************************************************************/
public:
	const QLCInputDevice* device() const;

protected:
	QLCInputDevice* m_device;
};

#endif
