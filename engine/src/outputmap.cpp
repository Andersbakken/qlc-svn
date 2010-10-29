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
#include <QByteArray>
#include <QSettings>
#include <QString>
#include <QDebug>
#include <QList>
#include <QtXml>
#include <QDir>

#ifdef __APPLE__
#include <QCoreApplication>
#endif

#include "qlcoutplugin.h"
#include "qlcconfig.h"
#include "qlctypes.h"

#include "dummyoutplugin.h"
#include "outputpatch.h"
#include "outputmap.h"

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

OutputMap::OutputMap(QObject* parent, quint32 universes) : QObject(parent)
{
    m_universes = universes;
    m_dummyOut = NULL;
    m_blackout = false;
    m_universeChanged = false;

    m_universeArray = new QByteArray(512 * universes, 0);

    initPatch();
}

OutputMap::~OutputMap()
{
    delete m_universeArray;
    m_universeArray = NULL;

    for (quint32 i = 0; i < m_universes; i++)
    {
        delete m_patch[i];
        m_patch[i] = NULL;
    }

    while (m_plugins.isEmpty() == false)
        delete m_plugins.takeFirst();

    /* The purge above gets rid of m_dummyOut as well. Just NULL it. */
    m_dummyOut = NULL;
}

void OutputMap::loadPlugins(const QString& path)
{
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
        /* Attempt to load a plugin from the path */
        QString fileName(it.next());
        QString path = dir.absoluteFilePath(fileName);
        QPluginLoader loader(path, this);
        QLCOutPlugin* p = qobject_cast<QLCOutPlugin*> (loader.instance());
        if (p != NULL)
        {
            /* Check for duplicates */
            if (plugin(p->name()) == NULL)
            {
                /* New plugin. Append and init. */
                qDebug() << "Output plugin" << p->name() << "from" << fileName;
                p->init();
                appendPlugin(p);
            }
            else
            {
                /* Duplicate plugin. Unload it. */
                qWarning() << "Discarded duplicate output plugin" << path;
                loader.unload();
            }
        }
        else
        {
            qWarning() << fileName << "doesn't contain a QLC output plugin:"
                       << loader.errorString();
            loader.unload();
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
        QByteArray zeros(512, 0);
        for (quint32 i = 0; i < m_universes; i++)
            m_patch[i]->dump(zeros);
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

QByteArray* OutputMap::claimUniverses()
{
    m_universeMutex.lock();
    return m_universeArray;
}

void OutputMap::releaseUniverses()
{
    m_universeChanged = true;
    m_universeMutex.unlock();
}

void OutputMap::dumpUniverses()
{
    m_universeMutex.lock();
    if (m_universeChanged == true && m_blackout == false)
    {
        for (quint32 i = 0; i < m_universes; i++)
            m_patch[i]->dump(m_universeArray->mid(i * 512, 512));

        m_universeChanged = false;
    }
    m_universeMutex.unlock();
}

uchar OutputMap::value(quint32 channel) const
{
    if (channel < quint32(m_universes * 512))
        return (*m_universeArray)[channel];
    else
        return 0;
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

    for (quint32 i = 0; i < m_universes; i++)
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

quint32 OutputMap::universes() const
{
    return m_universes;
}

bool OutputMap::setPatch(quint32 universe, const QString& pluginName,
                         quint32 output)
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

OutputPatch* OutputMap::patch(quint32 universe) const
{
    if (universe < KUniverseCount)
        return m_patch[universe];
    else
        return NULL;
}

QStringList OutputMap::universeNames() const
{
    QStringList list;
    for (quint32 i = 0; i < KUniverseCount; i++)
    {
        OutputPatch* p(patch(i));
        Q_ASSERT(p != NULL);
        list << QString("%1: %2 (%3)").arg(i + 1)
                                      .arg(p->pluginName())
                                      .arg(p->outputName());
    }

    return list;
}

quint32 OutputMap::mapping(const QString& pluginName, quint32 output) const
{
    for (quint32 uni = 0; uni < universes(); uni++)
    {
        const OutputPatch* p = patch(uni);
        if (p->pluginName() == pluginName && p->output() == output)
            return uni;
    }

    return KChannelInvalid;
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
    if (outputPlugin != NULL)
        outputPlugin->configure();
}

bool OutputMap::canConfigurePlugin(const QString& pluginName)
{
    QLCOutPlugin* outputPlugin = plugin(pluginName);
    if (outputPlugin != NULL)
        return outputPlugin->canConfigure();
    else
        return false;
}

QString OutputMap::pluginStatus(const QString& pluginName, quint32 output)
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
        m_plugins.append(outputPlugin);
        connect(outputPlugin, SIGNAL(configurationChanged()),
                this, SLOT(slotConfigurationChanged()));
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

void OutputMap::slotConfigurationChanged()
{
    QLCOutPlugin* plugin = qobject_cast<QLCOutPlugin*> (QObject::sender());
    if (plugin != NULL)
        emit pluginConfigurationChanged(plugin->name());
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

    for (quint32 i = 0; i < KUniverseCount; i++)
    {
        /* Zero-based addressing */
        key = QString("/outputmap/universe%1/dmxzerobased").arg(i);
        QVariant value = settings.value(key);
        if (value.isValid() == true)
            m_patch[i]->setDMXZeroBased(value.toBool());

        /* Plugin name */
        key = QString("/outputmap/universe%2/plugin/").arg(i);
        plugin = settings.value(key).toString();

        /* Plugin output */
        key = QString("/outputmap/universe%2/output/").arg(i);
        output = settings.value(key).toString();

        if (plugin.length() > 0 && output.length() > 0)
        {
            /* Check that the same plugin & output are not mapped
               to more than one universe at a time. */
            quint32 m = mapping(plugin, output.toInt());
            if (m == KChannelInvalid || m == i)
                setPatch(i, plugin, output.toInt());
        }
    }
}

void OutputMap::saveDefaults()
{
    QSettings settings;
    QString key;
    QString str;

    for (quint32 i = 0; i < KUniverseCount; i++)
    {
        OutputPatch* outputPatch = patch(i);
        Q_ASSERT(outputPatch != NULL);

        /* Zero-based DMX addressing */
        key = QString("/outputmap/universe%1/dmxzerobased").arg(i);
        settings.setValue(key, outputPatch->isDMXZeroBased());

        /* Plugin name */
        key = QString("/outputmap/universe%2/plugin/").arg(i);
        settings.setValue(key, outputPatch->pluginName());

        /* Plugin output */
        key = QString("/outputmap/universe%2/output/").arg(i);
        settings.setValue(key, str.setNum(outputPatch->output()));
    }
}
