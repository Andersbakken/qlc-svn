/*
  Q Light Controller
  monitorfixture.h

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

#ifndef MONITORFIXTURE_H
#define MONITORFIXTURE_H

#include <QFrame>
#include <QList>
#include <QHash>

#include "common/qlctypes.h"

class QGridLayout;
class QFrame;
class QLabel;

class MonitorFixture : public QFrame
{
	Q_OBJECT

public:
	MonitorFixture(QWidget* parent = NULL);
	virtual ~MonitorFixture();

	/** Less-than operator for qSort() */
	bool operator<(const MonitorFixture& mof);

	/********************************************************************
	 * Fixture
	 ********************************************************************/
public:
	enum ChannelStyle
	{
		RelativeChannels,
		DMXChannels
	};

	void setFixture(t_fixture_id fxi_id);
	t_fixture_id fixture() const { return m_fixture; }

public slots:
	void slotSetChannelStyle(MonitorFixture::ChannelStyle style);
	void slotFixtureChanged(t_fixture_id fxi_id);
	void slotFixtureRemoved(t_fixture_id fxi_id);

protected:
	t_fixture_id m_fixture;
	ChannelStyle m_channelStyle;

	QLabel* m_fixtureLabel;
	QList <QLabel*> m_channelLabels;

	/********************************************************************
	 * Values
	 ********************************************************************/
public:
	enum ValueStyle
	{
		DMXValues,
		PercentageValues
	};

	void setValue(t_channel universeChannel, t_value value);

public slots:
	void slotSetValueStyle(MonitorFixture::ValueStyle style);

protected slots:
	void slotChangedValues(const QHash <t_channel,t_value>& values);

protected:
	ValueStyle m_valueStyle;
	QList <QLabel*> m_valueLabels;
};

#endif
