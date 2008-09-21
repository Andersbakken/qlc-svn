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
#include "common/qlctypes.h"
#include "ui_inputpatcheditor.h"

class InputMap;
class InputPatch;
class QStringList;

class InputPatchEditor : public QDialog, public Ui_InputPatchEditor
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	InputPatchEditor(QWidget* parent, InputMap* inputMap,
			 t_input_universe universe, const QString& pluginName,
			 t_input input);
	~InputPatchEditor();

protected:
	/** Initialize the tree to contain available plugins & their inputs */
	void fillTree();
	
	/** Initialize the combo to contain available device templates */
	void fillCombo();

protected slots:
	void slotCurrentItemChanged(QTreeWidgetItem* item);
	void slotDeviceComboActivated(const QString& name);

protected:
	/** Our input map that contains the actual mapping */
	InputMap* m_inputMap;

	/** The input universe being edited */
	t_input_universe m_universe;

	/********************************************************************
	 * Selection extraction
	 ********************************************************************/
public:
	const QString pluginName() const { return m_pluginName; }
	const QString inputName() const { return m_inputName; }
	t_input input() const { return m_input; }

protected:
	/** The selected plugin's name */
	QString m_pluginName;

	/** The selected input name */
	QString m_inputName;
	
	/** The selected input line number */
	t_input m_input;
};

#endif /* INPUTPATCHEDITOR_H */
