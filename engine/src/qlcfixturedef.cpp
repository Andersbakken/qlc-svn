/*
  Q Light Controller
  qlcfixturedef.cpp

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

#include <iostream>
#include <QString>
#include <QFile>
#include <QtXml>

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "qlcfile.h"

QLCFixtureDef::QLCFixtureDef()
{
    m_type = QString("Dimmer");
}

QLCFixtureDef::QLCFixtureDef(const QLCFixtureDef* fixtureDef)
{
    m_type = QString("Dimmer");

    if (fixtureDef != NULL)
        *this = *fixtureDef;
}

QLCFixtureDef::~QLCFixtureDef()
{
    while (m_channels.isEmpty() == false)
        delete m_channels.takeFirst();

    while (m_modes.isEmpty() == false)
        delete m_modes.takeFirst();
}

QLCFixtureDef& QLCFixtureDef::operator=(const QLCFixtureDef& fixture)
{
    if (this != &fixture)
    {
        QListIterator <QLCChannel*> chit(fixture.m_channels);
        QListIterator <QLCFixtureMode*> modeit(fixture.m_modes);

        m_manufacturer = fixture.m_manufacturer;
        m_model = fixture.m_model;
        m_type = fixture.m_type;

        /* Clear all channels */
        while (m_channels.isEmpty() == false)
            delete m_channels.takeFirst();

        /* Copy channels from the other fixture */
        while (chit.hasNext() == true)
            m_channels.append(new QLCChannel(chit.next()));

        /* Clear all modes */
        while (m_modes.isEmpty() == false)
            delete m_modes.takeFirst();

        /* Copy modes from the other fixture */
        while (modeit.hasNext() == true)
            m_modes.append(new QLCFixtureMode(this, modeit.next()));
    }

    return *this;
}

/****************************************************************************
 * General properties
 ****************************************************************************/

QString QLCFixtureDef::name() const
{
    return m_manufacturer + QString(" ") + m_model;
}

void QLCFixtureDef::setManufacturer(const QString& mfg)
{
    m_manufacturer = mfg;
}

QString QLCFixtureDef::manufacturer() const
{
    return m_manufacturer;
}

void QLCFixtureDef::setModel(const QString& model)
{
    m_model = model;
}

QString QLCFixtureDef::model() const
{
    return m_model;
}

void QLCFixtureDef::setType(const QString& type)
{
    m_type = type;
}

QString QLCFixtureDef::type() const
{
    return m_type;
}

/****************************************************************************
 * Channels
 ****************************************************************************/

bool QLCFixtureDef::addChannel(QLCChannel* channel)
{
    if (channel != NULL && m_channels.contains(channel) == false)
    {
        m_channels.append(channel);
        return true;
    }
    else
    {
        return false;
    }
}

bool QLCFixtureDef::removeChannel(QLCChannel* channel)
{
    /* First remove the channel from all modes */
    QListIterator <QLCFixtureMode*> modeit(m_modes);
    while (modeit.hasNext() == true)
        modeit.next()->removeChannel(channel);

    /* Then remove the actual channel from this fixture definition */
    QMutableListIterator <QLCChannel*> chit(m_channels);
    while (chit.hasNext() == true)
    {
        if (chit.next() == channel)
        {
            chit.remove();
            delete channel;
            return true;
        }
    }

    return false;
}

QLCChannel* QLCFixtureDef::channel(const QString& name)
{
    QListIterator <QLCChannel*> it(m_channels);
    QLCChannel* ch = NULL;

    while (it.hasNext() == true)
    {
        ch = it.next();
        if (ch->name() == name)
            return ch;
    }

    return NULL;
}

QList <QLCChannel*> QLCFixtureDef::channels() const
{
    return m_channels;
}

/****************************************************************************
 * Modes
 ****************************************************************************/

bool QLCFixtureDef::addMode(QLCFixtureMode* mode)
{
    if (mode != NULL && m_modes.contains(mode) == false)
    {
        m_modes.append(mode);
        return true;
    }
    else
    {
        return false;
    }
}

bool QLCFixtureDef::removeMode(QLCFixtureMode* mode)
{
    QMutableListIterator <QLCFixtureMode*> it(m_modes);
    while (it.hasNext() == true)
    {
        if (it.next() == mode)
        {
            it.remove();
            delete mode;
            return true;
        }
    }

    return false;
}

const QLCFixtureMode* QLCFixtureDef::mode(const QString& name) const
{
    QListIterator <QLCFixtureMode*> it(m_modes);
    QLCFixtureMode* mode = NULL;

    while (it.hasNext() == true)
    {
        mode = it.next();
        if (mode->name() == name)
            return mode;
    }

    return NULL;
}

QList <QLCFixtureMode*> QLCFixtureDef::modes() const
{
    return m_modes;
}

/****************************************************************************
 * XML operations
 ****************************************************************************/

QFile::FileError QLCFixtureDef::saveXML(const QString& fileName)
{
    QFile::FileError error;

    if (fileName.isEmpty() == true)
        return QFile::OpenError;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly) == false)
        return file.error();

    QDomDocument doc(QLCFile::getXMLHeader(KXMLQLCFixtureDefDocument));
    Q_ASSERT(doc.isNull() == false);

    /* Create a text stream for the file */
    QTextStream stream(&file);

    /* Fixture tag */
    QDomElement root = doc.documentElement();

    QDomElement tag;
    QDomText text;

    /* Manufacturer */
    tag = doc.createElement(KXMLQLCFixtureDefManufacturer);
    root.appendChild(tag);
    text = doc.createTextNode(m_manufacturer);
    tag.appendChild(text);

    /* Model */
    tag = doc.createElement(KXMLQLCFixtureDefModel);
    root.appendChild(tag);
    text = doc.createTextNode(m_model);
    tag.appendChild(text);

    /* Type */
    tag = doc.createElement(KXMLQLCFixtureDefType);
    root.appendChild(tag);
    text = doc.createTextNode(m_type);
    tag.appendChild(text);

    /* Channels */
    QListIterator <QLCChannel*> chit(m_channels);
    while (chit.hasNext() == true)
        chit.next()->saveXML(&doc, &root);

    /* Modes */
    QListIterator <QLCFixtureMode*> modeit(m_modes);
    while (modeit.hasNext() == true)
        modeit.next()->saveXML(&doc, &root);

    /* Write the document into the stream */
    stream << doc.toString();
    error = QFile::NoError;
    file.close();

    return error;
}

QFile::FileError QLCFixtureDef::loadXML(const QString& fileName)
{
    QFile::FileError error = QFile::NoError;

    if (fileName.isEmpty() == true)
        return QFile::OpenError;

    QDomDocument doc;
    doc = QLCFile::readXML(fileName);
    if (doc.isNull() == true)
    {
        qDebug() << "Unable to read from" << fileName;
        return QFile::ReadError;
    }

    if (doc.doctype().name() == KXMLQLCFixtureDefDocument)
    {
        if (loadXML(&doc) == true)
            error = QFile::NoError;
        else
            error = QFile::ReadError;
    }
    else
    {
        error = QFile::ReadError;
        qWarning() << fileName << "is not a fixture definition file";
    }

    return error;
}

bool QLCFixtureDef::loadXML(const QDomDocument* doc)
{
    QDomElement root;
    QDomNode node;
    QDomElement tag;
    bool retval = false;

    Q_ASSERT(doc);

    root = doc->documentElement();
    if (root.tagName() == KXMLQLCFixtureDef)
    {
        node = root.firstChild();
        while (node.isNull() == false)
        {
            tag = node.toElement();

            if (tag.tagName() == KXMLQLCCreator)
            {
            }
            else if (tag.tagName() == KXMLQLCFixtureDefManufacturer)
            {
                setManufacturer(tag.text());
            }
            else if (tag.tagName() == KXMLQLCFixtureDefModel)
            {
                setModel(tag.text());
            }
            else if (tag.tagName() == KXMLQLCFixtureDefType)
            {
                setType(tag.text());
            }
            else if (tag.tagName() == KXMLQLCChannel)
            {
                QLCChannel* ch = new QLCChannel();
                if (ch->loadXML(&tag) == true)
                {
                    /* Loading succeeded */
                    if (addChannel(ch) == false)
                    {
                        /* Channel already exists */
                        delete ch;
                    }
                }
                else
                {
                    /* Loading failed */
                    delete ch;
                }
            }
            else if (tag.tagName() == KXMLQLCFixtureMode)
            {
                QLCFixtureMode* mode = new QLCFixtureMode(this);
                if (mode->loadXML(&tag) == true)
                {
                    /* Loading succeeded */
                    if (addMode(mode) == false)
                    {
                        /* Mode already exists */
                        delete mode;
                    }
                }
                else
                {
                    /* Loading failed */
                    delete mode;
                }
            }
            else
                qDebug() << "Unknown Fixture tag: "
                << tag.tagName();

            node = node.nextSibling();
        }

        retval = true;
    }
    else
    {
        qDebug() << "Fixture node not found in file!";
        retval = false;
    }

    return retval;
}
