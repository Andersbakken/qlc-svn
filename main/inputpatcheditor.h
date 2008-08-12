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
#include "ui_inputpatcheditor.h"

class InputMap;
class InputPatch;
class QStringList;

class InputPatchEditor : public QDialog, public Ui_InputPatchEditor
{
	Q_OBJECT

public:
	InputPatchEditor(QWidget* parent, InputMap* inputMap, int universe,
		       const QString& pluginName, int input);
	~InputPatchEditor();

	const QString pluginName() const { return m_pluginName; }
	int input() const { return m_input; }

protected slots:
	void slotPluginActivated(const QString& pluginName);
	void slotInputActivated(int line);

protected:
	InputMap* m_inputMap;
	QStringList m_pluginList;

	int m_universe;

	QString m_pluginName;
	int m_input;
};

#endif /* INPUTPATCHEDITOR_H */
