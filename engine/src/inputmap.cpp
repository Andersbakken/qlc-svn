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

#include <QCoreApplication>
#include <QPluginLoader>
#include <QStringList>
#include <QSettings>
#include <QDebug>
#include <QList>
#include <QtXml>
#include <QDir>

#ifdef WIN32
#   include <windows.h>
#endif

#include "qlcinputchannel.h"
#include "qlcinplugin.h"
#include "qlcconfig.h"
#include "qlctypes.h"
#include "qlcfile.h"
#include "qlci18n.h"

#include "inputpatch.h"
#include "inputmap.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

InputMap::InputMap(QObject*parent, quint32 universes) : QObject(parent)
{
    m_universes = universes;
    m_editorUniverse = 0;

    initPatch();
}

InputMap::~InputMap()
{
    /* Clear patching table so that when it gets out of scope AFTER this
       destructor is run, it won't attempt to do close() on already-deleted
       plugin pointers. */
    for (quint32 i = 0; i < m_universes; i++)
    {
        delete m_patch[i];
        m_patch[i] = NULL;
    }

    while (m_plugins.isEmpty() == false)
        delete m_plugins.takeFirst();

    while (m_profiles.isEmpty() == false)
        delete m_profiles.takeFirst();
}

/*****************************************************************************
 * Universes
 *****************************************************************************/

quint32 InputMap::invalidUniverse()
{
    return UINT_MAX;
}

quint32 InputMap::universes() const
{
    return m_universes;
}

quint32 InputMap::editorUniverse() const
{
    return m_editorUniverse;
}

void InputMap::setEditorUniverse(quint32 uni)
{
    if (uni < universes())
        m_editorUniverse = uni;
    else
        m_editorUniverse = 0;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void InputMap::slotValueChanged(quint32 input, quint32 channel, uchar value)
{
    QLCInPlugin* plugin = qobject_cast<QLCInPlugin*> (QObject::sender());
    if (plugin == NULL)
        return;

    for (quint32 i = 0; i < m_universes; i++)
    {
        if (m_patch[i]->plugin() == plugin &&
                m_patch[i]->input() == input)
        {
            emit inputValueChanged(i, channel, value);
        }
    }
}

bool InputMap::feedBack(quint32 universe, quint32 channel, uchar value)
{
    if (universe >= quint32(m_patch.size()))
        return false;

    InputPatch* patch = m_patch[universe];
    Q_ASSERT(patch != NULL);

    if (patch->plugin() != NULL && patch->feedbackEnabled() == true)
    {
        patch->plugin()->feedBack(patch->input(), channel, value);
        return true;
    }
    else
    {
        return false;
    }
}

void InputMap::slotConfigurationChanged()
{
    QLCInPlugin* plugin = qobject_cast<QLCInPlugin*> (QObject::sender());
    if (plugin != NULL)
        emit pluginConfigurationChanged(plugin->name());
}

/*****************************************************************************
 * Patch
 *****************************************************************************/

void InputMap::initPatch()
{
    for (quint32 i = 0; i < m_universes; i++)
        m_patch.insert(i, new InputPatch(this));
}

bool InputMap::setPatch(quint32 universe, const QString& pluginName,
                        quint32 input, bool enableFeedback,
                        const QString& profileName)
{
    /* Check that the universe that we're doing mapping for is valid */
    if (universe >= m_universes)
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return false;
    }

    /* Don't care if plugin or profile is NULL. It must be possible to
       clear the patch completely. */
    m_patch[universe]->set(plugin(pluginName), input, enableFeedback,
                           profile(profileName));

    return true;
}

InputPatch* InputMap::patch(quint32 universe) const
{
    if (universe < m_universes)
        return m_patch[universe];
    else
        return NULL;
}

quint32 InputMap::mapping(const QString& pluginName, quint32 input) const
{
    for (quint32 uni = 0; uni < universes(); uni++)
    {
        const InputPatch* p = patch(uni);
        if (p->pluginName() == pluginName && p->input() == input)
            return uni;
    }

    return InputMap::invalidUniverse();
}

/*****************************************************************************
 * Plugins
 *****************************************************************************/

void InputMap::loadPlugins(const QDir& dir)
{
    /* Check that we can access the directory */
    if (dir.exists() == false || dir.isReadable() == false)
        return;

    /* Loop thru all files in the directory */
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        /* Attempt to load a plugin from the path */
        QString fileName(it.next());
        QString path = dir.absoluteFilePath(fileName);
        QPluginLoader loader(path, this);
        QLCInPlugin* p = qobject_cast<QLCInPlugin*> (loader.instance());
        if (p != NULL)
        {
            /* Check for duplicates */
            if (plugin(p->name()) == NULL)
            {
                /* New plugin. Append and init. */
                qDebug() << "Input plugin" << p->name() << "from" << fileName;
                p->init();
                appendPlugin(p);
                QLCi18n::loadTranslation(p->name().replace(" ", "_"));
            }
            else
            {
                /* Duplicate plugin. Unload it. */
                qWarning() << Q_FUNC_INFO << "Discarded duplicate input plugin"
                           << fileName;
                loader.unload();
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << fileName
                       << "doesn't contain a QLC input plugin:"
                       << loader.errorString();
            loader.unload();
        }
    }
}

QStringList InputMap::pluginNames()
{
    QListIterator <QLCInPlugin*> it(m_plugins);
    QStringList list;

    while (it.hasNext() == true)
        list.append(it.next()->name());

    return list;
}

QStringList InputMap::pluginInputs(const QString& pluginName)
{
    QLCInPlugin* ip = plugin(pluginName);
    if (ip == NULL)
        return QStringList();
    else
        return ip->inputs();
}

void InputMap::configurePlugin(const QString& pluginName)
{
    QLCInPlugin* inputPlugin = plugin(pluginName);
    if (inputPlugin != NULL)
        inputPlugin->configure();
}

bool InputMap::canConfigurePlugin(const QString& pluginName)
{
    QLCInPlugin* inputPlugin = plugin(pluginName);
    if (inputPlugin != NULL)
        return inputPlugin->canConfigure();
    else
        return false;
}

QString InputMap::pluginStatus(const QString& pluginName, quint32 input)
{
    QLCInPlugin* inputPlugin = NULL;
    QString info;

    if (pluginName.isEmpty() == false)
        inputPlugin = plugin(pluginName);

    if (inputPlugin != NULL)
    {
        info = inputPlugin->infoText(input);
    }
    else
    {
        /* Nothing selected */
        info += QString("<HTML><HEAD></HEAD><BODY>");
        info += QString("<H3>%1</H3>").arg(tr("No plugin selected"));
        info += QString("<P>%1 ").arg(tr("You can download plugins from"));
        info += QString("<A HREF=\"http://www.sourceforge.net/projects/qlc/files\">");
        info += QString("http://www.sourceforge.net/projects/qlc/files</A></P>.");
        info += QString("</BODY></HTML>");
    }

    return info;
}

bool InputMap::appendPlugin(QLCInPlugin* inputPlugin)
{
    Q_ASSERT(inputPlugin != NULL);

    if (plugin(inputPlugin->name()) == NULL)
    {
        m_plugins.append(inputPlugin);
        connect(inputPlugin, SIGNAL(configurationChanged()),
                this, SLOT(slotConfigurationChanged()));
        connect(inputPlugin, SIGNAL(valueChanged(quint32,quint32,uchar)),
                this, SLOT(slotValueChanged(quint32,quint32,uchar)));
        emit pluginAdded(inputPlugin->name());
        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Input plugin" << inputPlugin->name()
                   << "is already loaded. Skipping.";
        return false;
    }
}

QLCInPlugin* InputMap::plugin(const QString& name)
{
    QListIterator <QLCInPlugin*> it(m_plugins);

    while (it.hasNext() == true)
    {
        QLCInPlugin* plugin = it.next();
        if (plugin->name() == name)
            return plugin;
    }

    return NULL;
}

QDir InputMap::systemPluginDirectory()
{
    QDir dir;
#ifdef __APPLE__
    dir.setPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                                   .arg(INPUTPLUGINDIR));
#else
    dir.setPath(INPUTPLUGINDIR);
#endif

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtPlugin));

    return dir;
}

/*****************************************************************************
 * Profiles
 *****************************************************************************/

void InputMap::loadProfiles(const QDir& dir)
{
    if (dir.exists() == false || dir.isReadable() == false)
        return;

    /* Go thru all found file entries and attempt to load an input
       profile from each of them. */
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        QLCInputProfile* prof;
        QString path;

        path = dir.absoluteFilePath(it.next());
        prof = QLCInputProfile::loader(path);
        if (prof != NULL)
        {
            /* Check for duplicates */
            if (profile(prof->name()) == NULL)
                addProfile(prof);
            else
                delete prof;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unable to find an input profile from" << path;
        }
    }
}

QStringList InputMap::profileNames()
{
    QStringList list;
    QListIterator <QLCInputProfile*> it(m_profiles);
    while (it.hasNext() == true)
        list << it.next()->name();
    return list;
}

QLCInputProfile* InputMap::profile(const QString& name)
{
    QListIterator <QLCInputProfile*> it(m_profiles);
    while (it.hasNext() == true)
    {
        QLCInputProfile* profile = it.next();
        if (profile->name() == name)
            return profile;
    }

    return NULL;
}

bool InputMap::addProfile(QLCInputProfile* profile)
{
    Q_ASSERT(profile != NULL);

    /* Don't add the same profile twice */
    if (m_profiles.contains(profile) == false)
    {
        m_profiles.append(profile);
        return true;
    }
    else
    {
        return false;
    }
}

bool InputMap::removeProfile(const QString& name)
{
    QLCInputProfile* profile;
    QMutableListIterator <QLCInputProfile*> it(m_profiles);
    while (it.hasNext() == true)
    {
        profile = it.next();
        if (profile->name() == name)
        {
            it.remove();
            delete profile;
            return true;
        }
    }

    return false;
}

bool InputMap::inputSourceNames(quint32 universe, quint32 channel,
                                QString& uniName, QString& chName) const
{
    if (universe == InputMap::invalidUniverse() ||
        channel == KInputChannelInvalid)
    {
        /* Nothing given for input universe and/or channel */
        return false;
    }

    InputPatch* pat = this->patch(universe);
    if (pat == NULL || pat->plugin() == NULL)
    {
        /* There is no patch for the given universe */
        return false;
    }

    QLCInputProfile* profile = pat->profile();
    if (profile == NULL)
    {
        /* There is no profile. Display plugin name and channel number. */
        uniName = tr("%1: %2").arg(universe + 1).arg(pat->plugin()->name());
        chName = tr("%1: Unknown").arg(channel + 1);
    }
    else
    {
        QLCInputChannel* ich;
        QString name;

        /* Display profile name for universe */
        uniName = QString("%1: %2").arg(universe + 1).arg(profile->name());

        /* User can input the channel number by hand, so put something
           rational to the channel name in those cases as well. */
        ich = profile->channel(channel);
        if (ich != NULL)
            name = ich->name();
        else
            name = tr("Unknown");

        /* Display channel name */
        chName = QString("%1: %2").arg(channel + 1).arg(name);
    }

    return true;
}

QDir InputMap::systemProfileDirectory()
{
    QDir dir;

#ifdef __APPLE__
    dir.setPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                              .arg(INPUTPROFILEDIR));
#else
    dir.setPath(INPUTPROFILEDIR);
#endif

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtInputProfile));
    return dir;
}

QDir InputMap::userProfileDirectory()
{
    QDir dir;

#ifdef Q_WS_X11
    // If the current user is root, return the system profile dir.
    // Otherwise return the user's home dir.
    if (geteuid() == 0)
        dir = QDir(INPUTPROFILEDIR);
    else
        dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(USERINPUTPROFILEDIR));
#elif __APPLE__
    /* User's input profile directory on OSX */
    dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(USERINPUTPROFILEDIR));
#else
    /* User's input profile directory on Windows */
    LPTSTR home = (LPTSTR) malloc(256 * sizeof(TCHAR));
    GetEnvironmentVariable(TEXT("UserProfile"), home, 256);
    dir.setPath(QString("%1/%2")
                    .arg(QString::fromUtf16(reinterpret_cast<ushort*> (home)))
                    .arg(USERINPUTPROFILEDIR));
    free(home);
#endif

    /* Ensure that the selected profile directory exists */
    if (dir.exists() == false)
        dir.mkpath(".");

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtInputProfile));
    return dir;
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void InputMap::loadDefaults()
{
    bool feedbackEnabled;
    QString profileName;
    QSettings settings;
    QString plugin;
    QVariant value;
    QString input;
    QString key;

    /* Editor universe */
    key = QString("/inputmap/editoruniverse/");
    value = settings.value(key);
    if (value.isValid() == true)
        setEditorUniverse(value.toInt());

    for (quint32 i = 0; i < m_universes; i++)
    {
        /* Plugin name */
        key = QString("/inputmap/universe%2/plugin/").arg(i);
        plugin = settings.value(key).toString();

        /* Plugin input */
        key = QString("/inputmap/universe%2/input/").arg(i);
        input = settings.value(key).toString();

        /* Input profile */
        key = QString("/inputmap/universe%2/profile/").arg(i);
        profileName = settings.value(key).toString();

        /* Feedback enable */
        key = QString("/inputmap/universe%2/feedbackEnabled/").arg(i);
        if (settings.value(key).isValid() == true)
            feedbackEnabled = settings.value(key).toBool();
        else
            feedbackEnabled = true;

        /* Do the mapping */
        if (plugin.length() > 0 && input.length() > 0)
        {
            /* Check that the same plugin & input are not mapped
               to more than one universe at a time. */
            quint32 m = mapping(plugin, input.toInt());
            if (m == InputMap::invalidUniverse() || m == i)
            {
                setPatch(i, plugin, input.toInt(),
                         feedbackEnabled, profileName);
            }
        }
    }
}

void InputMap::saveDefaults()
{
    QSettings settings;
    QString key;
    QString str;

    for (quint32 i = 0; i < m_universes; i++)
    {
        InputPatch* pat = patch(i);
        Q_ASSERT(pat != NULL);

        /* Editor universe */
        key = QString("/inputmap/editoruniverse/");
        settings.setValue(key, m_editorUniverse);

        if (pat->plugin() != NULL)
        {
            /* Plugin name */
            key = QString("/inputmap/universe%2/plugin/").arg(i);
            settings.setValue(key, pat->plugin()->name());

            /* Plugin input */
            key = QString("/inputmap/universe%2/input/").arg(i);
            settings.setValue(key, str.setNum(pat->input()));

            /* Input profile */
            key = QString("/inputmap/universe%2/profile/").arg(i);
            settings.setValue(key, pat->profileName());

            /* Feedback enable */
            key = QString("/inputmap/universe%2/feedbackEnabled/").arg(i);
            settings.setValue(key, pat->feedbackEnabled());
        }
        else
        {
            /* Plugin name */
            key = QString("/inputmap/universe%2/plugin/").arg(i);
            settings.setValue(key, "");

            /* Plugin input */
            key = QString("/inputmap/universe%2/input/").arg(i);
            settings.setValue(key, "");

            /* Input profile */
            key = QString("/inputmap/universe%2/profile/").arg(i);
            settings.setValue(key, "");

            /* Feedback enable */
            key = QString("/inputmap/universe%2/feedbackEnabled/").arg(i);
            settings.setValue(key, true);
        }
    }
}
