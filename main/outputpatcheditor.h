/*
  Q Light Controller
  outputpatcheditor.h

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

#ifndef OUTPUTPATCHEDITOR_H
#define OUTPUTPATCHEDITOR_H

#include <QDialog>

#include "ui_outputpatcheditor.h"
#include "common/qlctypes.h"

class OutputMap;
class OutputPatch;
class QStringList;

class OutputPatchEditor : public QDialog, public Ui_OutputPatchEditor
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	OutputPatchEditor(QWidget* parent, int universe,
			  const QString& pluginName, t_output output);
	~OutputPatchEditor();

protected:
	void fillTree();
	void updateOutputInfo();

protected slots:
	void slotCurrentItemChanged(QTreeWidgetItem* item);
	void slotConfigureClicked();

protected:
	/** The output universe being edited */
	int m_universe;

	/********************************************************************
	 * Selection extraction
	 ********************************************************************/
public:
	const QString pluginName() const { return m_pluginName; }
	const QString outputName() const { return m_outputName; }
	t_output output() const { return m_output; }

protected:
	/** The selected plugin's name */
	QString m_pluginName;

	/** The selected output name */
	QString m_outputName;
	
	/** The selected output line number */
	t_output m_output;
};

#endif /* OUTPUTPATCHEDITOR_H */
