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

#include <QDebug>
#include <QDir>

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
	if (m_models.contains(manufacturer) == false)
	{
		return NULL;
	}
	else
	{
		QMap <QString, QLCFixtureDef*> defs(m_models[manufacturer]);
		if (defs.contains(model) == false)
		{
			return NULL;
		}
		else
		{
			return defs[model];
		}
	}
}

QStringList QLCFixtureDefCache::manufacturers() const
{
	return m_models.keys();
}

QStringList QLCFixtureDefCache::models(const QString& manufacturer) const
{
	if (m_models.contains(manufacturer) == true)
	{
		QMap <QString, QLCFixtureDef*> defs(m_models[manufacturer]);
		return defs.keys();
	}
	else
	{
		return QStringList();
	}
}

bool QLCFixtureDefCache::addFixtureDef(QLCFixtureDef* fixtureDef)
{
	Q_ASSERT(fixtureDef != NULL);

	QMap <QString, QLCFixtureDef*> defs(
					m_models[fixtureDef->manufacturer()]);
	/* Don't accept duplicate entries */
	if (defs.contains(fixtureDef->model()) == true)
		return false;

	/* Add the fixture definition to the manufacturer's map and emit it */
	m_models[fixtureDef->manufacturer()][fixtureDef->model()] = fixtureDef;

	return true;
}

bool QLCFixtureDefCache::load(const QString& fixturePath)
{
	QDir dir(fixturePath, QString("*%1").arg(KExtFixture),
		 QDir::Name, QDir::Files);
	if (dir.exists() == false || dir.isReadable() == false)
	{
		qWarning() << "Unable to load fixture definitions from"
			   << fixturePath;
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
			qWarning() << "Fixture definition loading from"
				   << path << "failed: "
				   << QLCFile::errorString(error);
			delete fxi;
			fxi = NULL;
		}
	}

	return true;
}

void QLCFixtureDefCache::clear()
{
	QMutableMapIterator <QString, QMap <QString, QLCFixtureDef*> > it(m_models);
	while (it.hasNext() == true)
	{
		it.next();

		QMutableMapIterator <QString, QLCFixtureDef*> mit(it.value());
		while (mit.hasNext() == true)
		{
			mit.next();
			delete mit.value();
			mit.remove();
		}

		it.remove();
	}
}
