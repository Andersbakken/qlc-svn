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

#include <QCoreApplication>
#include <QList>
#include <QDebug>
#include <QSet>

#ifdef WIN32
#   include <windows.h>
#endif

#include "qlcfixturedefcache.h"
#include "qlcfixturedef.h"
#include "qlcconfig.h"
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
        qWarning() << Q_FUNC_INFO << "Cache already contains"
                   << fixtureDef->name();
        return false;
    }
}

bool QLCFixtureDefCache::load(const QDir& dir)
{
    if (dir.exists() == false || dir.isReadable() == false)
        return false;

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
            qWarning() << Q_FUNC_INFO << "Fixture definition loading from"
                       << path << "failed:" << QLCFile::errorString(error);
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

QDir QLCFixtureDefCache::systemDefinitionDirectory()
{
    QDir dir;
#ifdef __APPLE__
    dir.setPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                                   .arg(FIXTUREDIR));
#else
    dir.setPath(FIXTUREDIR);
#endif

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));

    return dir;
}

QDir QLCFixtureDefCache::userDefinitionDirectory()
{
    QDir dir;

#ifdef Q_WS_X11
    // If the current user is root, return the system fixture dir.
    // Otherwise return a path under user's home dir.
    if (geteuid() == 0)
        dir = QDir(FIXTUREDIR);
    else
        dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(USERFIXTUREDIR));
#elif __APPLE__
    /* User's input profile directory on OSX */
    dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(USERFIXTUREDIR));
#else
    /* User's input profile directory on Windows */
    LPTSTR home = (LPTSTR) malloc(256 * sizeof(TCHAR));
    GetEnvironmentVariable(TEXT("UserProfile"), home, 256);
    dir.setPath(QString("%1/%2")
                    .arg(QString::fromUtf16(reinterpret_cast<ushort*> (home)))
                    .arg(USERFIXTUREDIR));
    free(home);
#endif

    // Ensure the directory exists
    if (dir.exists() == false)
        dir.mkpath(".");

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));

    return dir;
}
