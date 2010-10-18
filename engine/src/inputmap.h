/*
  Q Light Controller
  inputmap.h

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

#ifndef INPUTMAP_H
#define INPUTMAP_H

#include <QObject>
#include <QVector>
#include <QList>

#include "qlcinputprofile.h"
#include "qlctypes.h"

class QLCInPlugin;
class InputPatch;
class InputMap;

class QDomDocument;
class QDomElement;

#define KInputNone QObject::tr("None")

#define KXMLQLCInputMap "InputMap"
#define KXMLQLCInputMapEditorUniverse "EditorUniverse"

class InputMap : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(InputMap)

    friend class InputPatch;
    friend class InputMapEditor;
    friend class InputPatchEditor;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /**
     * Create a new InputMap object, with the given amount of input
     * universes.
     */
    InputMap(QObject* parent,
             quint32 universes = KInputUniverseCount);

    /**
     * Destroy an InputMap object
     */
    virtual ~InputMap();

    /*********************************************************************
     * Input data
     *********************************************************************/
public slots:
    /** Slot that catches input plugins' value changes */
    void slotValueChanged(quint32 input, quint32 channel, uchar value);

    /** Slot that catches plugin configuration change notifications */
    void slotConfigurationChanged();

public:
    /** Send feedback value to the input profile e.g. to move a motorized
        sliders & knobs, set indicator leds etc. */
    bool feedBack(quint32 universe, quint32 channel,
                  uchar value);

signals:
    /** Everyone interested in input data should connect to this signal */
    void inputValueChanged(quint32 universe,
                           quint32 channel, uchar value);

    /** Notifies (InputManager) of plugin configuration changes */
    void pluginConfigurationChanged(const QString& pluginName);

    /*********************************************************************
     * Patch
     *********************************************************************/
public:
    /**
     * Get the number of supported input universes
     */
    quint32 universes() const;

    /**
     * Get the universe that is used for editing functions etc.
     */
    quint32 editorUniverse() const;

    /**
     * Set the universe that is used for editing functions etc.
     */
    void setEditorUniverse(quint32 uni);

    /**
     * Patch the given universe to go thru the given plugin
     *
     * @param universe The input universe to patch
     * @param pluginName The name of the plugin to patch to the universe
     * @param input An input universe provided by the plugin to patch to
     * @param enableFeedback enable/disable feedback data sending
     * @param profileName The name of an input profile
     * @return true if successful, otherwise false
     */
    bool setPatch(quint32 universe, const QString& pluginName,
                  quint32 input, bool enableFeedback,
                  const QString& profileName = QString::null);

    /**
     * Get mapping for an input universe.
     *
     * @param universe The internal input universe to get mapping for
     */
    InputPatch* patch(quint32 universe) const;

    /**
     * Check, whether a certain input in a certain plugin has been mapped
     * to a universe. Returns the mapped universe number or -1 if not
     * mapped.
     *
     * @param pluginName The name of the plugin to check for
     * @param input The particular input to check for
     * @return Mapped universe number or -1 if not mapped
     */
    quint32 mapping(const QString& pluginName, quint32 input) const;

protected:
    /** Initialize the patch table */
    void initPatch();

protected:
    /** Vector containing all active input plugins and the internal
        universes that they are associated to. */
    QVector <InputPatch*> m_patch;

    /** Total number of supported input universes */
    quint32 m_universes;

    /** The universe used to edit functions etc. */
    quint32 m_editorUniverse;

    /*********************************************************************
     * Plugins
     *********************************************************************/
public:
    /**
     * Load all input plugins from the input plugin directory
     */
    void loadPlugins();

    /**
     * Get a list of available input plugins as a string list
     * containing the plugins' names
     *
     * @return QStringList containing plugins' names
     */
    QStringList pluginNames();

    /**
     * Get the names of all input lines provided by the given plugin.
     *
     * @param pluginName Name of the plugin, whose input count to get
     * @return A QStringList containing the names of each input line
     *
     */
    QStringList pluginInputs(const QString& pluginName);

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
     * Get a status text for the given plugin.
     *
     * @param pluginName Name of the plugin, whose status to get
     * @param input A specific input identifier
     */
    QString pluginStatus(const QString& pluginName = QString::null,
                         quint32 input = KInputInvalid);

    /**
     * Append the given plugin to our list of plugins. Will fail if
     * a plugin with the same name already exists.
     *
     * @param inputPlugin The input plugin to append
     * @return true if successful, otherwise false
     */
    bool appendPlugin(QLCInPlugin* inputPlugin);

protected:
    /**
     * Get a plugin instance by the plugin's name
     *
     * @param name The name of the plugin to search for
     * @return QLCInPlugin or NULL
     */
    QLCInPlugin* plugin(const QString& name);

protected:
    /** List containing all available input plugins */
    QList <QLCInPlugin*> m_plugins;

    /*********************************************************************
     * Input profiles
     *********************************************************************/
public:
    /** Load all profile profiles from the given path */
    void loadProfiles(const QString& profilePath);

    /** Get a list of available profile names */
    QStringList profileNames();

    /** Get a profile by its name */
    QLCInputProfile* profile(const QString& name);

    /** Add a new profile */
    bool addProfile(QLCInputProfile* profile);

    /** Remove an existing profile by its name and delete it */
    bool removeProfile(const QString& name);

protected:
    /** List that contains all available profiles */
    QList <QLCInputProfile*> m_profiles;

    /*********************************************************************
     * Defaults
     *********************************************************************/
public:
    /**
     * Load default settings for input mapper from QLC global settings
     */
    void loadDefaults();

    /**
     * Save default settings for input mapper into QLC global settings
     */
    void saveDefaults();
};

#endif
