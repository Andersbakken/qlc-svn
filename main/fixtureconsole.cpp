/*
  Q Light Controller
  fixtureconsole.cpp

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

#include <QHBoxLayout>
#include <QDebug>
#include <QIcon>
#include <QtXml>

#include "common/qlcfile.h"

#include "fixtureconsole.h"
#include "consolechannel.h"
#include "app.h"
#include "doc.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FixtureConsole::FixtureConsole(QWidget* parent, t_fixture_id fxi_id)
	: QWidget(parent)
{
	new QHBoxLayout(this);

	setFixture(fxi_id);
}

FixtureConsole::~FixtureConsole()
{
}

/*****************************************************************************
 * Fixture
 *****************************************************************************/

void FixtureConsole::setFixture(t_fixture_id id)
{
	ConsoleChannel* cc = NULL;
	Fixture* fxi = NULL;
	
	m_fixture = id;

	fxi = _app->doc()->fixture(m_fixture);
	Q_ASSERT(fxi != NULL);

	/* Create channel units */
	for (unsigned int i = 0; i < fxi->channels(); i++)
	{
		cc = new ConsoleChannel(this, m_fixture, i);
		layout()->addWidget(cc);

		connect(cc, SIGNAL(valueChanged(t_channel,t_value,bool)),
			this, SLOT(slotValueChanged(t_channel,t_value,bool)));
	}

	/* Resize the console to some sensible proportions if at least
	   one channel unit was inserted */
	if (cc != NULL)
		resize((fxi->channels() * cc->width()), 250);
}

/*****************************************************************************
 * Channels
 *****************************************************************************/

void FixtureConsole::enableAllChannels(bool enable)
{
	/* All of this' children are ConsoleChannel objects, except layout */
	QListIterator <QObject*> it(children());
	while (it.hasNext() == true)
	{
		ConsoleChannel* cc = qobject_cast<ConsoleChannel*> (it.next());
		if (cc != NULL)
			cc->enable(enable);
	}
}

void FixtureConsole::setOutputDMX(bool state)
{
	/* All of this' children are ConsoleChannel objects, except layout */
	QListIterator <QObject*> it(children());
	while (it.hasNext() == true)
	{
		ConsoleChannel* cc = qobject_cast<ConsoleChannel*> (it.next());
		if (cc != NULL)
			cc->setOutputDMX(state);
	}
}

void FixtureConsole::setSceneValue(const SceneValue& scv)
{
	Q_ASSERT(scv.fxi == m_fixture);

	QListIterator <QObject*> it(children());
	while (it.hasNext() == true)
	{
		ConsoleChannel* cc = qobject_cast<ConsoleChannel*> (it.next());
		if (cc != NULL && cc->channel() == scv.channel)
		{
			cc->enable(true);
			cc->setValue(scv.value);
		}
	}
}

void FixtureConsole::slotValueChanged(t_channel channel, t_value value,
				      bool enabled)
{
	emit valueChanged(m_fixture, channel, value, enabled);
}

/*****************************************************************************
 * Save / Load
 *****************************************************************************/

bool FixtureConsole::loadXML(QDomDocument*, QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	bool visible = false;

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFixtureConsole)
	{
		qWarning() << "Fixture console node not found!";
		return false;
	}

	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCWindowState)
		{
			QLCFile::loadXMLWindowState(&tag, &x, &y, &w, &h,
						    &visible);
		}
		else
		{
			qDebug() << "Unknown fixture console tag:"
			     << tag.tagName();
		}
		
		node = node.nextSibling();
	}

	hide();
	setGeometry(x, y, w, h);
	if (visible == false)
		showMinimized();
	else
		showNormal();

	return true;
}

bool FixtureConsole::saveXML(QDomDocument* doc, QDomElement* fxi_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(fxi_root != NULL);

	/* Fixture Console entry */
	root = doc->createElement(KXMLQLCFixtureConsole);
	fxi_root->appendChild(root);

	/* Save window state */
	return QLCFile::saveXMLWindowState(doc, &root, parentWidget());
}
