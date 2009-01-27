/*
  Q Light Controller
  dummyoutplugin.h

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

#ifndef DUMMYOUTPLUGIN_H
#define DUMMYOUTPLUGIN_H

#include <QObject>
#include "common/qlcoutplugin.h"
#include "common/qlctypes.h"

class DummyOutPlugin : public QObject, public QLCOutPlugin
{
	Q_OBJECT
	Q_INTERFACES(QLCOutPlugin)

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	DummyOutPlugin();
	virtual ~DummyOutPlugin();

	void init();

private:
	Q_DISABLE_COPY(DummyOutPlugin)

	/*********************************************************************
	 * Open/close
	 *********************************************************************/
public:
	void open(t_output output = 0);
	void close(t_output output = 0);
	QStringList outputs();

	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	QString name();

	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	void configure();

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	QString infoText(t_output output = KOutputInvalid);

	/*********************************************************************
	 * Value Read/Write
	 *********************************************************************/
public:
	void writeChannel(t_output output, t_channel channel, t_value value);
	void writeRange(t_output output, t_channel address, t_value* values,
			t_channel num);

	void readChannel(t_output output, t_channel channel, t_value* value);
	void readRange(t_output output, t_channel address, t_value* values,
		       t_channel num);
};

#endif
