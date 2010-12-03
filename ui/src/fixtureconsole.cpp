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
#include <QList>
#include <QtXml>

#include "qlcfile.h"

#include "fixtureconsole.h"
#include "consolechannel.h"
#include "inputmap.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FixtureConsole::FixtureConsole(QWidget* parent) : QWidget(parent)
{
    m_fixture = Fixture::invalidId();
    m_channelsCheckable = false;
    m_externalInputEnabled = false;

    new QHBoxLayout(this);
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

    /* Get rid of any previous channels */
    while (m_channels.isEmpty() == false)
        delete m_channels.takeFirst();

    m_fixture = id;

    fxi = _app->doc()->fixture(m_fixture);
    Q_ASSERT(fxi != NULL);

    /* Create channel units */
    for (unsigned int i = 0; i < fxi->channels(); i++)
    {
        const QLCChannel* ch = fxi->channel(i);
        Q_ASSERT(ch != NULL);
        if (ch->group() == QLCChannel::NoGroup)
            continue;

        cc = new ConsoleChannel(this, m_fixture, i);
        cc->setCheckable(m_channelsCheckable);
        layout()->addWidget(cc);

        connect(cc, SIGNAL(valueChanged(quint32,uchar,bool)),
                this, SLOT(slotValueChanged(quint32,uchar,bool)));

        m_channels.append(cc);
    }

    /* Make a spacer item eat excess space to justify channels left */
    layout()->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding));
}

/*****************************************************************************
 * Channels
 *****************************************************************************/

void FixtureConsole::setChannelsCheckable(bool checkable)
{
    m_channelsCheckable = checkable;

    QMutableListIterator <ConsoleChannel*> it(m_channels);
    while (it.hasNext() == true)
        it.next()->setCheckable(checkable);
}

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

void FixtureConsole::slotValueChanged(quint32 channel, uchar value,
                                      bool enabled)
{
    emit valueChanged(m_fixture, channel, value, enabled);
}

QList <SceneValue> FixtureConsole::values() const
{
    QList <SceneValue> list;

    QListIterator <QObject*> it(children());
    while (it.hasNext() == true)
    {
        ConsoleChannel* cc;

        cc = qobject_cast<ConsoleChannel*> (it.next());
        if (cc != NULL && cc->isEnabled() == true)
        {
            list.append(SceneValue(m_fixture, cc->channel(),
                                   cc->sliderValue()));
        }
    }

    return list;
}

void FixtureConsole::setValues(const QList <SceneValue>& list)
{
    enableAllChannels(false);

    QListIterator <SceneValue> it(list);
    while (it.hasNext() == true)
    {
        SceneValue val(it.next());
        if (val.channel < quint32(children().size()))
        {
            ConsoleChannel* cc = channel(val.channel);
            if (cc != NULL)
            {
                cc->enable(true);
                cc->setValue(val.value);
            }
        }
    }
}

ConsoleChannel* FixtureConsole::channel(quint32 ch)
{
    QListIterator <QObject*> it(children());
    while (it.hasNext() == true)
    {
        ConsoleChannel* cc;
        cc = qobject_cast<ConsoleChannel*> (it.next());
        if (cc != NULL && cc->channel() == ch)
            return cc;
    }

    return NULL;
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void FixtureConsole::enableExternalInput(bool enable)
{
    if (enable == true && m_externalInputEnabled == false)
    {
        connect(_app->inputMap(),
                SIGNAL(inputValueChanged(quint32,
                                         quint32,
                                         uchar)),
                this, SLOT(slotInputValueChanged(quint32,
                                                 quint32,
                                                 uchar)));
        m_externalInputEnabled = true;
    }
    else if (enable == false && m_externalInputEnabled == true)
    {
        disconnect(_app->inputMap(),
                   SIGNAL(inputValueChanged(quint32,
                                            quint32,
                                            uchar)),
                   this,
                   SLOT(slotInputValueChanged(quint32,
                                              quint32,
                                              uchar)));
        m_externalInputEnabled = false;
    }
}

void FixtureConsole::slotInputValueChanged(quint32 uni,
        quint32 ch,
        uchar value)
{
    if (uni == _app->inputMap()->editorUniverse())
    {
        ConsoleChannel* cc;

        cc = channel(ch);
        if (cc != NULL)
        {
            cc->setValue(value);
            cc->setChecked(true);
        }
    }
}
