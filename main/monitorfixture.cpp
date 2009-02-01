/*
  Q Light Controller
  monitorfixture.cpp

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

#include <QGridLayout>
#include <QString>
#include <QFrame>
#include <QLabel>
#include <QDebug>
#include <QHash>
#include <cmath>

#include "common/qlctypes.h"
#include "monitorfixture.h"
#include "outputmap.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

MonitorFixture::MonitorFixture(QWidget* parent) : QFrame(parent)
{
	m_fixtureLabel = NULL;
	m_fixture = KNoID;
	m_channelStyle = RelativeChannels;
	m_valueStyle = DMXValues;

	new QGridLayout(this);
	layout()->setMargin(3);

	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	setAutoFillBackground(true);
	setBackgroundRole(QPalette::Window);

	/* Listen to existing fixture changes and removals */
	connect(_app->doc(), SIGNAL(fixtureChanged(t_fixture_id)),
		this, SLOT(slotFixtureChanged(t_fixture_id)));
	connect(_app->doc(), SIGNAL(fixtureRemoved(t_fixture_id)),
		this, SLOT(slotFixtureRemoved(t_fixture_id)));
}

MonitorFixture::~MonitorFixture()
{
	if (m_fixtureLabel != NULL)
		delete m_fixtureLabel;

	while (m_channelLabels.isEmpty() == false)
		delete m_channelLabels.takeFirst();
	while (m_valueLabels.isEmpty() == false)
		delete m_valueLabels.takeFirst();
}

bool MonitorFixture::operator<(const MonitorFixture& mof)
{
	Fixture* fxi;
	Fixture* mof_fxi;

	fxi = _app->doc()->fixture(m_fixture);
	Q_ASSERT(fxi != NULL);

	mof_fxi = _app->doc()->fixture(mof.fixture());
	Q_ASSERT(mof_fxi != NULL);

	if ((*fxi) < (*mof_fxi))
		return true;
	else
		return false;
}

/****************************************************************************
 * Fixture
 ****************************************************************************/

void MonitorFixture::setFixture(t_fixture_id fxi_id)
{
	Fixture* fxi;

	/* Get rid of old stuff first, if such exists */
	if (m_fixtureLabel != NULL)
		delete m_fixtureLabel;
	while (m_channelLabels.isEmpty() == false)
		delete m_channelLabels.takeFirst();
	while (m_valueLabels.isEmpty() == false)
		delete m_valueLabels.takeFirst();

	m_fixture = fxi_id;
	fxi = _app->doc()->fixture(m_fixture);
	if (fxi != NULL)
	{
		QGridLayout* lay;
		int i;

		/* The grid layout uses columns and rows. The first row is for
		   the fixture name, second row for channel numbers and the
		   third row for channel values. Each channel is in its own
		   column. */
		lay = qobject_cast<QGridLayout*> (layout());

		/* Create a new fixture label and set the fixture name there */
		m_fixtureLabel = new QLabel(this);
		m_fixtureLabel->setText(QString("<B>%1</B> <I>(Universe %2)</I>")
					.arg(fxi->name())
					.arg(fxi->universe() + 1));

		/* Set the fixture name to span all channels horizontally */
		lay->addWidget(m_fixtureLabel, 0, 0, 1, fxi->channels(),
				Qt::AlignLeft);

		/* Create channel numbers and value labels */
		for (i = 0; i < fxi->channels(); i++)
		{
			t_channel channel;
			t_value value;
			QLabel* label;
			QString str;

			/* Set channel number according to style */
			if (m_channelStyle == DMXChannels)
				channel = fxi->address() + i;
			else
				channel = i;

			/* Get the channel's current value */
			value = _app->outputMap()->value(
						fxi->universeAddress() + i);

			/* Create a label for channel number */
			label = new QLabel(this);
			label->setText(str.sprintf("<B>%.3d</B>", channel + 1));
			lay->addWidget(label, 1, i, Qt::AlignHCenter);
			m_channelLabels.append(label);

			/* Create a label for value */
			label = new QLabel(this);
			label->setText(str.sprintf("%.3d", value));
			lay->addWidget(label, 2, i, Qt::AlignHCenter);
			m_valueLabels.append(label);
		}
	}
}

void MonitorFixture::slotSetChannelStyle(MonitorFixture::ChannelStyle style)
{
	QString str;
	int i = 1;

	if (m_channelStyle == style)
		return;
	m_channelStyle = style;

	if (m_channelStyle == DMXChannels)
	{
		Fixture* fxi = _app->doc()->fixture(m_fixture);
		if (fxi != NULL)
			i = fxi->address() + 1;
	}

	QListIterator <QLabel*> it(m_channelLabels);
	while (it.hasNext() == true)
		it.next()->setText(str.sprintf("<B>%.3d</B>", i++));
}

void MonitorFixture::slotFixtureChanged(t_fixture_id fxi_id)
{
	/* Create this object's contents anew */
	if (fxi_id == m_fixture)
		setFixture(fxi_id);
}

void MonitorFixture::slotFixtureRemoved(t_fixture_id fxi_id)
{
	if (fxi_id == m_fixture)
		deleteLater(); /* Can't delete this immediately */
}

/****************************************************************************
 * Values
 ****************************************************************************/

void MonitorFixture::updateValues()
{
	QLabel* label;
	t_value value;
	Fixture* fxi;
	QString str;
	int i = 0;

	/* Check that this MonitorFixture represents a fixture */
	if (m_fixture == KNoID)
		return;

	/* Check that this MonitorFixture's fixture really exists */
	fxi = _app->doc()->fixture(m_fixture);
	Q_ASSERT(fxi != NULL);

	QListIterator <QLabel*> it(m_valueLabels);
	while (it.hasNext() == true)
	{
		label = it.next();
		Q_ASSERT(label != NULL);

		value = _app->outputMap()->value(fxi->universeAddress() + i);
		i++;

		/* Set the label's text to reflect the changed value */
		if (m_valueStyle == DMXValues)
		{
			label->setText(str.sprintf("%.3d", value));
		}
		else
		{
			label->setText(str.sprintf("%.3d", int(
				ceil(SCALE(double(value),
					   double(0), double(255),
					   double(0), double(100))))));
		}
	}
}

void MonitorFixture::slotSetValueStyle(MonitorFixture::ValueStyle style)
{
	if (m_valueStyle == style)
		return;
	m_valueStyle = style;

	QListIterator <QLabel*> it(m_valueLabels);
	while (it.hasNext() == true)
	{
		QLabel* label;
		QString str;
		int value;

		label = it.next();
		Q_ASSERT(label != NULL);

		value = label->text().toInt();

		if (m_valueStyle == DMXValues)
			value = int(ceil(SCALE(double(value),
						double(0), double(100),
						double(0), double(255))));
		else
			value = int(ceil(SCALE(double(value),
						double(0), double(255),
						double(0), double(100))));

		label->setText(str.sprintf("%.3d", value));
	}
}
