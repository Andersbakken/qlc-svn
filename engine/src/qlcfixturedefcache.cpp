/*
  Q Light Controller
  qlcfixturedefcache.cpp

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

#include <QList>
#include <QDebug>
#include <QDir>
#include <QSet>

#include "qlcfixturedefcache.h"
#include "qlcfixturedef.h"
#include "qlcfile.h"

QLCFixtureDefCache::QLCFixtureDefCache()
{
}

QLCFixtureDefCache::~QLCFixtureDefCache()
{
    clear();
}

const QLCFixtureDef* QLCFixtureDefCache::fixtureDef(
    const QString& manufacturer, const QString& model) const
{
    QListIterator <QLCFixtureDef*> it(m_defs);
    while (it.hasNext() == true)
    {
        const QLCFixtureDef* def = it.next();
        if (def->manufacturer() == manufacturer && def->model() == model)
            return def;
    }

    return NULL;
}

QStringList QLCFixtureDefCache::manufacturers() const
{
    QSet <QString> makers;

    // Gather a list of manufacturers
    QListIterator <QLCFixtureDef*> it(m_defs);
    while (it.hasNext() == true)
        makers << it.next()->manufacturer();

    // Bounce the QSet into a QStringList
    QStringList list;
    foreach (QString manuf, makers)
        list << manuf;

    return list;
}

QStringList QLCFixtureDefCache::models(const QString& manufacturer) const
{
    QSet <QString> models;
    QListIterator <QLCFixtureDef*> it(m_defs);
    while (it.hasNext() == true)
    {
        QLCFixtureDef* def = it.next();
        if (def->manufacturer() == manufacturer)
            models << def->model();
    }

    // Bounce the QSet into a QStringList
    QStringList list;
    foreach (QString manuf, models)
        list << manuf;

    return list;
}

bool QLCFixtureDefCache::addFixtureDef(QLCFixtureDef* fixtureDef)
{
    if (fixtureDef == NULL)
        return false;

    if (models(fixtureDef->manufacturer()).contains(fixtureDef->model()) == false)
    {
        m_defs << fixtureDef;
        return true;
    }
    else
    {
        qWarning() << "Cache already contains" << fixtureDef->name();
        return false;
    }
}

bool QLCFixtureDefCache::load(const QString& fixturePath)
{
    QDir dir(fixturePath, QString("*%1").arg(KExtFixture),
             QDir::Name, QDir::Files);
    if (dir.exists() == false || dir.isReadable() == false)
    {
        qWarning() << "Unable to load fixture definitions from" << fixturePath;
        return false;
    }

    /* Attempt to read all specified files from the given directory */
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        QLCFixtureDef* fxi;
        QString path;

        path = dir.absoluteFilePath(it.next());

        fxi = new QLCFixtureDef();
        Q_ASSERT(fxi != NULL);

        QFile::FileError error = fxi->loadXML(path);
        if (error == QFile::NoError)
        {
            /* Delete the def if it's a duplicate. */
            if (addFixtureDef(fxi) == false)
                delete fxi;
            fxi = NULL;
        }
        else
        {
            qWarning() << "Fixture definition loading from" << path
                       << "failed: " << QLCFile::errorString(error);
            delete fxi;
            fxi = NULL;
        }
    }

    return true;
}

void QLCFixtureDefCache::clear()
{
    while (m_defs.isEmpty() == false)
        delete m_defs.takeFirst();
}
