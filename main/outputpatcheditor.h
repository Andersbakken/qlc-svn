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

class QStringList;
class OutputPatch;
class OutputMap;

class OutputPatchEditor : public QDialog, public Ui_OutputPatchEditor
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	OutputPatchEditor(QWidget* parent, int universe,
			  const OutputPatch* patch);
	~OutputPatchEditor();

public slots:
	void reject();

private:
	Q_DISABLE_COPY(OutputPatchEditor)

protected:
	void fillTree();
	void updateOutputInfo();

protected slots:
	void slotCurrentItemChanged(QTreeWidgetItem* item);
	void slotItemChanged(QTreeWidgetItem* item);
	void slotConfigureClicked();

protected:
	int m_universe;

	QString m_originalPluginName;
	QString m_currentPluginName;
	t_output m_originalOutput;
	t_output m_currentOutput;
};

#endif /* OUTPUTPATCHEDITOR_H */
