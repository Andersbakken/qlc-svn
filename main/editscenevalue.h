/*
  Q Light Controller
  editscenevalue.h

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

#ifndef EDITSCENEVALUE_H
#define EDITSCENEVALUE_H

#include <QDialog>

#include "ui_editscenevalue.cpp"
#include "common/qlctypes.h"
#include "scene.h"

class QWidget;
class QLCChannel;

class EditSceneValue : public QDialog, public Ui_EditSceneValue
{
	Q_OBJECT

public:
	EditSceneValue(QWidget* parent, QLCChannel* ch,
		       SceneValue &currentValue);
	virtual ~EditSceneValue();

	t_value value() { return m_value; }
	QString type() { return m_type; }

protected slots:
	void slotValueChanged(int);
	void slotPresetComboActivated(const QString &);
	void slotTypeActivated(const QString &text);

protected:
	bool m_updateValue;

	QLCChannel* m_channel;

	t_value m_value;
	QString m_type;
};

#endif
