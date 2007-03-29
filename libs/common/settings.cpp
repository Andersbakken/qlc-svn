/*
  Q Light Controller
  settings.cpp

  Copyright (C) 2000, 2001, 2002 Heikki Junnila

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

#include <qfile.h>
#include <stdlib.h>
#include <qdir.h>
#include <qdom.h>

#include "settings.h"
#include "configitem.h"
#include "filehandler.h"

/**
 * Standard constructor
 *
 */
Settings::Settings()
{
}

/**
 * Standard destructor
 *
 */
Settings::~Settings()
{
	save();
	
	//
	// Delete all items from list
	while (m_items.isEmpty() == false)
	{
		delete m_items.take(0);
	}
}

/**
 * Set a "key = value" to config items
 *
 * @param key The name of the key to set
 * @param value The value to set
 */
int Settings::set(QString key, QString value)
{
	QPtrListIterator<ConfigItem> it(m_items);
	
	// Search for existing key
	while (it.current() != NULL)
	{
		if (it.current()->key() == key)
		{
			// Replace existing
			it.current()->setText(value);
			
			return m_items.count();
		}
		else
		{
			++it;
		}
	}
	
	//
	// If we come here, it means the key was not found so we must create new
	ConfigItem* item = new ConfigItem;
	item->setKey(key);
	item->setText(value);
	m_items.append(item);
	
	return m_items.count();
}

/**
 * Overloaded member that behaves just like the one above, but takes an int
 * as the value.
 *
 * @param key The name of the key to set
 * @param value The value to set
 */
int Settings::set(QString key, int value)
{
	QString str;
	str.setNum(value);
	
	return set(key, str);
}

/**
 * Get a "key = value" from config items.
 *
 * @param key The key to get
 * @param value The value of the key
 */
int Settings::get(QString key, QString &text)
{
	int result = -1;
	QPtrListIterator<ConfigItem> it(m_items);
	
	/* Search for the key and set &text to its value */
	while (it.current() != NULL)
	{
		if (it.current()->key() == key)
		{
			text = it.current()->text();
			result = 1;
			break;
		}

		++it;
	}
	
	return result;
}

/**
 * Remove a key (as well as its value) from config items and delete it
 *
 * @param key The key to remove
 */
int Settings::remove(QString key)
{
	QPtrListIterator<ConfigItem> it(m_items);
	int compare = INT_MAX;
	
	// Search for existing key
	while (it.current() != NULL &&
	       (compare = (QString::compare(it.current()->key(), key) != 0)))
	{
		++it;
	}
	
	if (compare == 0)
	{
		delete m_items.take();
	}
	else
	{
		return -1;
	}
	
	return m_items.count();
}


/**
 * Load settings from file
 */
void Settings::load()
{
	// Search for a file from user's home directory
	QString path = QString(getenv("HOME")) +
		QString("/") + QString(KQLCUserDir);

	QString fileName = path + QString("/") + QString(KConfigFile);
	QPtrList <QString> list;

	if (FileHandler::readFileToList(fileName, list) == true)
	{
		for (QString* s = list.first(); s != NULL; s = list.next())
		{
			if (*s == QString("Entry"))
			{
				if (*(list.next()) == QString("General"))
				{
					createContents(list);
				}
			}
		}
	}

	while (!list.isEmpty())
	{
		delete list.take(0);
	}
}

/*
 * Create contents from list created by FileHandler
 */
void Settings::createContents(QPtrList <QString> &list)
{
	for (QString* s = list.next(); s != NULL; s = list.next())
	{
		if (*s == QString("Entry"))
		{
			s = list.prev();
			break;
		}
		else
		{
			set(*s, *(list.next()));
		}
	}
}

/*
 * Save config list contents to settings file
 */
void Settings::save()
{
	QString path = QString(getenv("HOME")) + QString("/") + 
		QString(KQLCUserDir);
	QDir dir(path);
	
	if (dir.exists() == false)
	{
		dir.mkdir(path);
	}
	
	// Save file to user's home directory
	QString fileName = path + QString("/") + QString(KConfigFile);
	
	QFile file(fileName);
	if ( file.open(IO_WriteOnly | IO_Truncate) )
	{
		QString buf;
		
		// Comment header
		buf = QString("# QLC Config file\n");
		file.writeBlock(buf, buf.length());
		
		// Entry name
		buf = QString("Entry = General\n");
		file.writeBlock(buf, buf.length());
		
		QPtrListIterator<ConfigItem> it(m_items);
		
		// Search for existing key
		it.toFirst();
		while (it.current() != NULL)
		{
			buf = QString(it.current()->key()) + QString(" = ") +
				QString(it.current()->text()) + QString("\n");
			file.writeBlock(buf, buf.length());
			
			++it;
		}
	}
	
	file.close();
}

/*
 * Class ConfigItem implementation
 */
ConfigItem::ConfigItem()
{
}

ConfigItem::~ConfigItem()
{
}

void ConfigItem::setKey(const QString key)
{
	m_key = key;
}

void ConfigItem::setText(const QString text)
{
	m_text = text;
}

const QString ConfigItem::key() const
{
	return m_key;
}

const QString ConfigItem::text() const
{
	return m_text;
}
