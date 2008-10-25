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
			 InputPatch* patch);
	~InputPatchEditor();

protected slots:
	void accept();

protected:
	/** The input universe that is being edited */
	t_input_universe m_universe;

	/** The input patch that is being edited */
	InputPatch* m_inputPatch;

	/********************************************************************
	 * Mapping page
	 ********************************************************************/
protected:
	void setupMappingPage();
	void fillMappingTree();

protected slots:
	void slotMappingCurrentItemChanged(QTreeWidgetItem* item);
	void slotConfigureInputClicked();

	/********************************************************************
	 * Template page
	 ********************************************************************/
protected:
	void setupTemplatePage();
	void fillTemplateTree();
	void updateTemplateItem(const QString& name, QTreeWidgetItem* item);

protected slots:
	void slotTemplateItemChanged(QTreeWidgetItem* item, int column);
	void slotAddTemplateClicked();
	void slotRemoveTemplateClicked();
	void slotEditTemplateClicked();

	/********************************************************************
	 * Selection
	 ********************************************************************/
protected:
	/** Selected plugin */
	QString m_pluginName;

	/** Selected input */
	QString m_inputName;
	t_input m_input;
	
	/* Selected device template */
	QString m_templateName;
};

#endif /* INPUTPATCHEDITOR_H */
