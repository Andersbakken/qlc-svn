/*
  Q Light Controller
  dmxpatcheditor.h

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

#ifndef DMXPATCHEDITOR_H
#define DMXPATCHEDITOR_H

#include "uic_dmxpatcheditor.h"

class DMXMap;
class DMXPatch;
class QStringList;

class DMXPatchEditor : public UI_DMXPatchEditor
{
	Q_OBJECT

public:
	DMXPatchEditor(QWidget* parent, DMXMap* dmxMap, int universe,
		       const QString& pluginName, int output);
	~DMXPatchEditor();

	const QString pluginName() const { return m_pluginName; }
	const int output() const { return m_output; }

protected slots:
	void slotPluginActivated(const QString& pluginName);
	void slotOutputActivated(int line);

protected:
	DMXMap* m_dmxMap;
	QStringList m_pluginList;

	int m_universe;

	QString m_pluginName;
	int m_output;
};

#endif /* DMXPATCHEDITOR_H */
