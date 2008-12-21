/*
  Q Light Controller
  inputchanneleditor.h

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

#ifndef INPUTCHANNELEDITOR_H
#define INPUTCHANNELEDITOR_H

#include <QDialog>

#include "common/qlcinputchannel.h"
#include "ui_inputchanneleditor.h"
#include "common/qlctypes.h"

class QLCInputChannel;

class InputChannelEditor : public QDialog, public Ui_InputChannelEditor
{
	Q_OBJECT

public:
	InputChannelEditor(QWidget* parent, const QLCInputChannel* channel);
	virtual ~InputChannelEditor();

	t_input_channel channel() const { return m_channel; }
	QString name() const { return m_name; }
	QLCInputChannel::Type type() const { return m_type; }

protected slots:
	void slotNumberChanged(int number);
	void slotNameEdited(const QString& text);
	void slotTypeActivated(const QString& text);

protected:
	t_input_channel m_channel;
	QString m_name;
	QLCInputChannel::Type m_type;
};

#endif
