/*
  Q Light Controller
  inputmap.cpp
  
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

#include <qptrvector.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qmessagebox.h>

#include "app.h"
#include "inputmap.h"
#include "common/inputplugin.h"
#include "common/types.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

InputMap::InputMap()
{
}

InputMap::~InputMap()
{
	InputPlugin* inputPlugin = NULL;

	while ((inputPlugin = m_plugins.take(0)) != NULL)
		delete inputPlugin;
}

/*****************************************************************************
 * Plugins
 *****************************************************************************/

QStringList InputMap::pluginNames()
{
	QPtrListIterator<InputPlugin> it(m_plugins);
	QStringList list;

	while (it.current() != NULL)
	{
		list.append((*it)->name());
		++it;
	}	
	
	return list;
}

int InputMap::pluginInputs(const QString& pluginName)
{
	return 0;
}

void InputMap::configurePlugin(const QString& pluginName)
{
	InputPlugin* inputPlugin = plugin(pluginName);
	if (inputPlugin == NULL)
		QMessageBox::warning(_app,
				     QString("Unable to configure plugin"),
				     pluginName + QString(" not found!"));
	else
		inputPlugin->configure(_app);
}

QString InputMap::pluginStatus(const QString& pluginName)
{
	InputPlugin* inputPlugin = NULL;
	QString info;

	if (pluginName != QString::null)
		inputPlugin = plugin(pluginName);
	
	if (inputPlugin == NULL)
	{
		// HTML header
		info += QString("<HTML>");
		info += QString("<HEAD>");
		info += QString("<TITLE>Input mapping status</TITLE>");
		info += QString("</HEAD>");
		info += QString("<BODY>");

		// Mapping status title
		info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
		info += QString("<TR>");
		info += QString("<TD BGCOLOR=\"");
		info += _app->colorGroup().highlight().name();
		info += QString("\">");
		info += QString("<FONT COLOR=\"");
		info += _app->colorGroup().highlightedText().name();
		info += QString("\" SIZE=\"5\">");
		info += QString("Input mapping status");
		info += QString("</FONT>");
		info += QString("</TD>");
		info += QString("</TR>");
		info += QString("</TABLE>");

		info += QString("TODO...");
	}
	else
	{
		info = inputPlugin->infoText();
	}

	return info;
}

bool InputMap::appendPlugin(InputPlugin* inputPlugin)
{
	Q_ASSERT(inputPlugin != NULL);

	if (plugin(inputPlugin->name()) == NULL)
	{
		qDebug("Found input plugin: " + inputPlugin->name());
		m_plugins.append(inputPlugin);
		return true;
	}
	else
	{
		qDebug(inputPlugin->name() + QString(" is already loaded."));
		return false;
	}
}

InputPlugin* InputMap::plugin(const QString& name)
{
	QPtrListIterator<InputPlugin> it(m_plugins);

	while (it.current() != NULL)
	{
		if (it.current()->name() == name)
			return it.current();
		++it;
	}
	
	return NULL;
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void InputMap::loadDefaults(const QString& path)
{
}

void InputMap::saveDefaults(const QString& path)
{
}
