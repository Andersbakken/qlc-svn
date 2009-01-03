/*
  Q Light Controller
  inputpatcheditor.h

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

#ifndef INPUTPATCHEDITOR_H
#define INPUTPATCHEDITOR_H

#include <QDialog>

#include <common/qlcinputdevice.h>
#include <common/qlctypes.h>

#include "ui_inputpatcheditor.h"
#include "inputpatch.h"

class QStringList;
class InputPatch;
class InputMap;

class InputPatchEditor : public QDialog, public Ui_InputPatchEditor
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	InputPatchEditor(QWidget* parent, t_input_universe universe,
			 const InputPatch* patch);
	~InputPatchEditor();

private:
	Q_DISABLE_COPY(InputPatchEditor)

protected slots:
	void reject();
	void accept();

protected:
	/** The input universe that is being edited */
	t_input_universe m_universe;

	QString m_originalPluginName;
	QString m_currentPluginName;

	t_input m_originalInput;
	t_input m_currentInput;

	QString m_originalDeviceName;
	QString m_currentDeviceName;

	/********************************************************************
	 * Mapping page
	 ********************************************************************/
protected:
	void setupMappingPage();
	void fillMappingTree();

protected slots:
	void slotMapCurrentItemChanged(QTreeWidgetItem* item);
	void slotMapItemChanged(QTreeWidgetItem* item);
	void slotConfigureInputClicked();

	/********************************************************************
	 * Device page
	 ********************************************************************/
protected:
	void setupDevicePage();
	void fillDeviceTree();
	void updateDeviceItem(const QString& name, QTreeWidgetItem* item);

protected slots:
	void slotDeviceItemChanged(QTreeWidgetItem* item);
	void slotAddDeviceClicked();
	void slotRemoveDeviceClicked();
	void slotEditDeviceClicked();
};

#endif /* INPUTPATCHEDITOR_H */
