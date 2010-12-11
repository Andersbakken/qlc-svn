/*
  Q Light Controller
  scenevalue.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

#include "scenevalue.h"

SceneValue::SceneValue(t_fixture_id id, quint32 ch, uchar val)
    : fxi(id)
    , channel(ch)
    , value(val)
{
}

SceneValue::SceneValue(const SceneValue& scv)
    : fxi(scv.fxi)
    , channel(scv.channel)
    , value(scv.value)
{
}

SceneValue::~SceneValue()
{
}

bool SceneValue::isValid() const
{
    if (fxi == Fixture::invalidId())
        return false;
    else
        return true;
}

bool SceneValue::operator<(const SceneValue& scv) const
{
    if (fxi < scv.fxi)
    {
        return true;
    }
    else if (fxi == scv.fxi)
    {
        if (channel < scv.channel)
            return true;
        else
            return false;
    }
    else
    {
        return false;
    }
}

bool SceneValue::operator==(const SceneValue& scv) const
{
    if (fxi == scv.fxi && channel == scv.channel)
        return true;
    else
        return false;
}

bool SceneValue::loadXML(const QDomElement& tag)
{
    if (tag.tagName() != KXMLQLCSceneValue)
    {
        qWarning() << Q_FUNC_INFO << "Scene node not found";
        return false;
    }

    fxi = t_fixture_id(tag.attribute(KXMLQLCSceneValueFixture).toInt());
    if (fxi < 0 || fxi >= KFixtureArraySize)
        return false;

    channel = quint32(tag.attribute(KXMLQLCSceneValueChannel).toInt());
    value = uchar(tag.text().toUInt());

    return isValid();
}

bool SceneValue::saveXML(QDomDocument* doc, QDomElement* scene_root) const
{
    QDomElement tag;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(scene_root != NULL);

    /* Value tag and its attributes */
    tag = doc->createElement(KXMLQLCSceneValue);
    tag.setAttribute(KXMLQLCSceneValueFixture, fxi);
    tag.setAttribute(KXMLQLCSceneValueChannel, channel);
    scene_root->appendChild(tag);

    /* The actual value as node text */
    text = doc->createTextNode(QString("%1").arg(value));
    tag.appendChild(text);

    return true;
}
