/*
  Q Light Controller
  outputmap.h

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

#ifndef OUTPUTMAP_H
#define OUTPUTMAP_H

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QList>
#include <QHash>

#include "qlctypes.h"

class QDomDocument;
class QDomElement;
class QByteArray;
class QString;

class QLCOutPlugin;
class OutputMap;
class OutputPatch;
class OutputMapEditor;
class OutputPatchEditor;

#define KOutputNone QObject::tr("None")
#define KXMLQLCOutputMap "OutputMap"

class OutputMap : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputMap)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /**
     * Create a new OutputMap object
     *
     * @param universes Number of universes
     */
    OutputMap(QObject* parent, int universes = KUniverseCount);

    /**
     * Destroy a OutputMap object
     */
    ~OutputMap();

    /**
     * Load all output plugins from the given directory.
     *
     * @param path The directory to load plugins from
     */
    void loadPlugins(const QString& path);

protected:
    /** Total number of supported universes */
    int m_universes;

    /*********************************************************************
     * Blackout
     *********************************************************************/
public:
    /**
     * Toggle blackout between on and off.
     *
     * @return New blackout state (i.e. after toggling)
     */
    bool toggleBlackout();

    /**
     * Set blackout on or off
     *
     * @param blackout If true, set blackout ON, otherwise OFF
     */
    void setBlackout(bool blackout);

    /**
     * Get blackout state
     *
     * @return true if blackout is ON, otherwise false
     */
    bool blackout() const;

signals:
    /**
     * Signal that is sent when blackout state is changed.
     *
     * @param state true if blackout has been turned on, otherwise false
     */
    void blackoutChanged(bool state);

protected:
    /** Current blackout state */
    bool m_blackout;

    /*********************************************************************
     * Values
     *********************************************************************/
public:
    /**
     * Claim access to all universes. This is declared virtual to make
     * unit testing a bit easier.
     */
    virtual QByteArray* claimUniverses();

    /**
     * Release access to all universes. This is declared virtual to make
     * unit testing a bit easier.
     */
    virtual void releaseUniverses();

    /**
     * Write current universe array data to plugins, each universe within
     * the array to its assigned plugin.
     */
    void dumpUniverses();

    /**
     * Grab a read-only copy of the current universe array for
     * monitoring purposes.
     */
    QByteArray peekUniverses() const {
        return *m_universeArray;
    }

    /**
     * Read the value of a single channel
     *
     * @param channel The channel to read the value from
     * @return The value of the channel (0 if channel is out of bounds)
     */
    uchar value(t_channel channel) const;

protected:
    /** The values of all universes */
    QByteArray* m_universeArray;

    /** When true, universes are dumped. Otherwise not. */
    bool m_universeChanged;

    /** Mutex guarding m_universeArray */
    QMutex m_universeMutex;

    /*********************************************************************
     * Patch
     *********************************************************************/
protected:
    /**
     * Initialize the patching table
     */
    void initPatch();

public:
    /**
     * Get the total number of supported universes
     *
     * @return Universe count supported by QLC
     */
    int universes() const;

    /**
     * Patch the given universe to go thru the given plugin
     *
     * @param universe The universe to patch
     * @param pluginName The name of the plugin to patch to the universe
     * @param output A universe provided by the plugin to patch to
     * @return true if successful, otherwise false
     */
    bool setPatch(unsigned int universe, const QString& pluginName,
                  unsigned int output = 0);

    /**
     * Get the output mapping for a QLC universe.
     *
     * @param universe The internal universe to get mapping for
     */
    OutputPatch* patch(int universe) const;

    /**
     * Get a list of available universes.
     */
    QStringList universeNames() const;

    /**
     * Check, whether a certain output in a certain plugin has been mapped
     * to a universe. Returns the mapped universe number or -1 if not
     * mapped.
     *
     * @param pluginName The name of the plugin to check for
     * @param output The particular output to check for
     * @return Mapped universe number or -1 if not mapped
     */
    int mapping(const QString& pluginName, quint32 output) const;

protected:
    /** Vector containing all active plugins */
    QVector <OutputPatch*> m_patch;

    /*********************************************************************
     * Plugins
     *********************************************************************/
public:
    /**
     * Get a list of available Output output plugins as a string list
     * containing the plugins' names
     *
     * @return QStringList containing plugins' names
     */
    QStringList pluginNames();

    /**
     * Get the number of universes provided by the given plugin.
     *
     * @param pluginName Name of the plugin, whose output count to get
     * @return A list of output names provided by the plugin.
     */
    QStringList pluginOutputs(const QString& pluginName);

    /**
     * Open a configuration dialog for the given plugin
     *
     * @param pluginName Name of the plugin to configure
     */
    void configurePlugin(const QString& pluginName);

    /**
     * Check, whether a plugin provides additional configuration options.
     *
     * @param pluginName The name of the plugin to check from.
     * @return true if plugin can be configured. Otherwise false.
     */
    bool canConfigurePlugin(const QString& pluginName);

    /**
     * Get a status text for the given plugin. If no plugin name is
     * given, an overall mapping status of all universes is returned.
     *
     * @param pluginName Name of the plugin, whose status to get
     * @param output Plugin's output line for getting more specific info
     */
    QString pluginStatus(const QString& pluginName = QString::null,
                         quint32 output = KOutputInvalid);

    /**
     * Append the given plugin to our list of plugins. Will fail if
     * a plugin with the same name already exists.
     *
     * @param outputPlugin The output plugin to append
     * @return true if successful, otherwise false
     */
    bool appendPlugin(QLCOutPlugin* outputPlugin);

protected:
    /**
     * Get a plugin instance by the plugin's name
     *
     * @param name The name of the plugin to search for
     * @return QLCOutPlugin or NULL
     */
    QLCOutPlugin* plugin(const QString& name);

protected slots:
   /** Slot that catches plugin configuration change notifications */
    void slotConfigurationChanged();

signals:
    /** Notifies (OutputManager) of plugin configuration changes */
    void pluginConfigurationChanged(const QString& pluginName);

protected:
    /** The dummy out plugin that is used for unused universes */
    QLCOutPlugin* m_dummyOut;

    /** List containing all available plugins */
    QList <QLCOutPlugin*> m_plugins;

    /*********************************************************************
     * Defaults
     *********************************************************************/
public:
    /**
     * Load default settings for output mapper from QLC global settings
     */
    void loadDefaults();

    /**
     * Save default settings for output mapper into QLC global settings
     */
    void saveDefaults();
};

#endif
