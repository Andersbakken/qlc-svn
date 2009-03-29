/*
  Q Light Controller
  outputmap.cpp

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

#include <QPluginLoader>
#include <QMessageBox>
#include <QByteArray>
#include <QSettings>
#include <QString>
#include <QDebug>
#include <QList>
#include <QtXml>
#include <QDir>

#ifdef __APPLE__
#include <QApplication>
#endif

#include "common/qlcoutplugin.h"
#include "common/qlctypes.h"

#include "dummyoutplugin.h"
#include "outputmanager.h"
#include "outputpatch.h"
#include "outputmap.h"
#include "app.h"

extern App* _app;

#ifdef WIN32
	#define PLUGINEXT ".dll"
#elif __APPLE__
	#define PLUGINEXT ".dylib"
#else
	#define PLUGINEXT ".so"
#endif

/*****************************************************************************
 * Initialization
 *****************************************************************************/

OutputMap::OutputMap(QObject* parent, int universes) : QObject(parent)
{
	m_universes = universes;
	m_dummyOut = NULL;
	m_blackout = false;

	m_universeArray = new QByteArray(universes * 512, 0);

	initPatch();

	load();
	loadDefaults();
}

OutputMap::~OutputMap()
{
	saveDefaults();

	delete m_universeArray;
	m_universeArray = NULL;

	for (int i = 0; i < m_universes; i++)
		delete m_patch[i];

	while (m_plugins.isEmpty() == false)
		delete m_plugins.takeFirst();

	/* The purge above gets rid of m_dummyOut as well. Just NULL it. */
	m_dummyOut = NULL;
}

void OutputMap::load()
{
	QString path;

#ifdef __APPLE__
	path = QString("%1/%2").arg(QApplication::applicationDirPath())
				.arg(OUTPUTPLUGINDIR);
#else
	path = QString(OUTPUTPLUGINDIR);
#endif

	QDir dir(path, QString("*%1").arg(PLUGINEXT), QDir::Name, QDir::Files);

	/* Check that we can access the directory */
	if (dir.exists() == false || dir.isReadable() == false)
	{
		qWarning() << "Unable to load output plugins from" << path;
		return;
	}

	/* Loop thru all files in the directory */
	QStringListIterator it(dir.entryList());
	while (it.hasNext() == true)
	{
		QLCOutPlugin* p;
		QString path;

		/* Attempt to load a plugin from the path */
		path = dir.absoluteFilePath(it.next());
		QPluginLoader loader(path, this);
		p = qobject_cast<QLCOutPlugin*> (loader.instance());
		if (p != NULL)
		{
			/* Check for duplicates */
			if (plugin(p->name()) == NULL)
			{
				/* New plugin. Append and init. */
				p->init();
				appendPlugin(p);
			}
			else
			{
				/* Duplicate plugin. Unload it. */
				qWarning() << "Discarded duplicate plugin"
					   << path;
				loader.unload();
			}
		}
		else
		{
			qWarning() << "Unable to load an output plugin from" 
				   << path << "because:"
				   << loader.errorString();
		}
	}
}

/*****************************************************************************
 * Blackout
 *****************************************************************************/

bool OutputMap::toggleBlackout()
{
	if (m_blackout == true)
		setBlackout(false);
	else
		setBlackout(true);

	return m_blackout;
}

void OutputMap::setBlackout(bool blackout)
{
	/* Don't do blackout twice */
	if (m_blackout == blackout)
		return;
	m_blackout = blackout;

	if (blackout == true)
	{
		for (int i = 0; i < m_universes; i++)
		{
			char zeros[512];
			std::fill(zeros, zeros + 512, 0);

			m_patch[i]->dump(zeros);
		}
	}
	else
	{
		/* Force writing of values back to the plugins */
		m_universeChanged = true;
	}

	emit blackoutChanged(m_blackout);
}

bool OutputMap::blackout() const
{
	return m_blackout;
}

/*****************************************************************************
 * Values
 *****************************************************************************/

t_value OutputMap::value(t_channel channel)
{
	t_value value;

	if (channel > (m_universes * 512))
		return 0;

	value = m_universeArray->data()[channel];

	return value;
}

void OutputMap::setValue(t_channel channel, t_value value)
{
	if (channel > m_universes * 512)
		return;

	m_universeArray->data()[channel] = value;
	m_universeChanged = true;
}

QByteArray* OutputMap::claimUniverses()
{
	return m_universeArray;
}

void OutputMap::releaseUniverses()
{
	m_universeChanged = true;
}

void OutputMap::dumpUniverses()
{
	if (m_universeChanged == true && m_blackout == false)
	{
		for (int i = 0; i < m_universes; i++)
		{
			m_patch[i]->dump(m_universeArray->constData()
					 + (512 * i));
		}

		m_universeChanged = false;
	}
}

/*****************************************************************************
 * Patch
 *****************************************************************************/

void OutputMap::initPatch()
{
	/* Create a dummy output plugin and put it to the plugins list */
	m_dummyOut = new DummyOutPlugin();
	m_dummyOut->init();
	appendPlugin(m_dummyOut);

	for (int i = 0; i < m_universes; i++)
	{
		OutputPatch* outputPatch;

		/* The dummy output plugin provides always as many outputs
		   as QLC has supported universes. So, assign each of these
		   outputs, by default, to each universe */
		outputPatch = new OutputPatch(this);
		outputPatch->set(m_dummyOut, i);
		m_patch.insert(i, outputPatch);
	}
}

bool OutputMap::setPatch(unsigned int universe, const QString& pluginName,
		      unsigned int output)
{
	if (int(universe) >= m_patch.size())
	{
		qWarning() << "Universe number out of bounds:" << universe
			   << "Unable to set patch.";
		return false;
	}

	QLCOutPlugin* outputPlugin = plugin(pluginName);
	if (outputPlugin == NULL)
	{
		qWarning() << "Plugin" << pluginName << "for universe number"
			   << universe << "not found.";
		return false;
	}

	m_patch[universe]->set(outputPlugin, output);

	return true;
}

OutputPatch* OutputMap::patch(int universe)
{
	Q_ASSERT(universe >= 0 && universe < KUniverseCount);
	return m_patch[universe];
}

/*****************************************************************************
 * Plugins
 *****************************************************************************/

QStringList OutputMap::pluginNames()
{
	QListIterator <QLCOutPlugin*> it(m_plugins);
	QStringList list;

	while (it.hasNext() == true)
		list.append(it.next()->name());

	return list;
}

QStringList OutputMap::pluginOutputs(const QString& pluginName)
{
	QLCOutPlugin* op = NULL;

	op = plugin(pluginName);
	if (op == NULL)
		return QStringList();
	else
		return op->outputs();
}

void OutputMap::configurePlugin(const QString& pluginName)
{
	QLCOutPlugin* outputPlugin = plugin(pluginName);
	if (outputPlugin == NULL)
		QMessageBox::warning(_app, tr("Unable to configure plugin"),
				     tr("%1 not found").arg(pluginName));
	else
		outputPlugin->configure();
}

QString OutputMap::pluginStatus(const QString& pluginName, t_output output)
{
	QLCOutPlugin* outputPlugin = NULL;
	QString info;

	if (pluginName != QString::null)
		outputPlugin = plugin(pluginName);

	if (outputPlugin != NULL)
		info = outputPlugin->infoText(output);
	else
		info = tr("No information");

	return info;
}

bool OutputMap::appendPlugin(QLCOutPlugin* outputPlugin)
{
	Q_ASSERT(outputPlugin != NULL);

	if (plugin(outputPlugin->name()) == NULL)
	{
		qDebug() << "Found an output plugin:" << outputPlugin->name();
		m_plugins.append(outputPlugin);
		return true;
	}
	else
	{
		qWarning() << "Output plugin" << outputPlugin->name()
			   << "is already loaded. Skipping";
		return false;
	}
}

QLCOutPlugin* OutputMap::plugin(const QString& name)
{
	QListIterator <QLCOutPlugin*> it(m_plugins);

	while (it.hasNext() == true)
	{
		QLCOutPlugin* plugin = it.next();
		if (plugin->name() == name)
			return plugin;
	}

	return NULL;
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool OutputMap::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement root;
	int i = 0;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* OutputMap entry */
	root = doc->createElement(KXMLQLCOutputMap);
	wksp_root->appendChild(root);

	/* Patches */
	for (i = 0; i < m_patch.size(); i++)
		m_patch[i]->saveXML(doc, &root, i);

	return true;
}

bool OutputMap::loadXML(const QDomElement* root)
{
	QDomNode node;
	QDomElement tag;

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCOutputMap)
	{
		qWarning() << "OutputMap node not found!";
		return false;
	}

	/* Patches */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCOutputPatch)
			OutputPatch::loader(&tag, this);
		else
			qWarning() << "Unknown OutputMap tag:" << tag.tagName();
		
		node = node.nextSibling();
	}

	return true;
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void OutputMap::loadDefaults()
{
	QSettings settings;
	QString plugin;
	QString output;
	QString key;

	for (int i = 0; i < KUniverseCount; i++)
	{
		/* Plugin name */
		key = QString("/outputmap/universe%2/plugin/").arg(i);
		plugin = settings.value(key).toString();

		/* Plugin output */
		key = QString("/outputmap/universe%2/output/").arg(i);
		output = settings.value(key).toString();

		if (plugin.length() > 0 && output.length() > 0)
			setPatch(i, plugin, output.toInt());
	}
}

void OutputMap::saveDefaults()
{
	QSettings settings;
	QString key;
	QString str;

	for (int i = 0; i < KUniverseCount; i++)
	{
		OutputPatch* outputPatch = patch(i);
		Q_ASSERT(outputPatch != NULL);
		Q_ASSERT(outputPatch->plugin() != NULL);

		/* Plugin name */
		key = QString("/outputmap/universe%2/plugin/").arg(i);
		settings.setValue(key, outputPatch->pluginName());

		/* Plugin output */
		key = QString("/outputmap/universe%2/output/").arg(i);
		settings.setValue(key, str.setNum(outputPatch->output()));
	}
}
