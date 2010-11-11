/*
  Q Light Controller
  inputpatcheditor.cpp

  Copyright (C) Heikki Junnila

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

#include <QTreeWidgetItem>
#include <QButtonGroup>
#include <QTreeWidget>
#include <QToolButton>
#include <QMessageBox>
#include <QSettings>
#include <QComboBox>
#include <QGroupBox>
#include <QVariant>
#include <QDebug>
#include <QFile>
#include <QDir>

#include "qlcinputprofile.h"
#include "qlcconfig.h"
#include "qlctypes.h"
#include "qlcfile.h"

#include "inputprofileeditor.h"
#include "inputpatcheditor.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "apputil.h"
#include "app.h"

extern App* _app;

#define SETTINGS_GEOMETRY "inputpatcheditor/geometry"

/* Plugin column structure */
#define KMapColumnName  0
#define KMapColumnInput 1

/* Profile column structure */
#define KProfileColumnName 0

InputPatchEditor::InputPatchEditor(QWidget* parent, quint32 universe,
                                   const InputPatch* inputPatch)
        : QDialog(parent)
{
    Q_ASSERT(universe < _app->inputMap()->universes());
    Q_ASSERT(inputPatch != NULL);

    setupUi(this);
    m_infoBrowser->setOpenExternalLinks(true);

    m_universe = universe;
    setWindowTitle(tr("Mapping properties for input universe %1")
                   .arg(m_universe + 1));

    /* Copy these so they can be applied if the user cancels */
    m_originalPluginName = inputPatch->pluginName();
    m_currentPluginName = inputPatch->pluginName();

    m_originalInput = inputPatch->input();
    m_currentInput = inputPatch->input();

    m_originalProfileName = inputPatch->profileName();
    m_currentProfileName = inputPatch->profileName();

    m_originalFeedbackEnabled = inputPatch->feedbackEnabled();
    m_currentFeedbackEnabled = inputPatch->feedbackEnabled();

    /* Setup UI controls */
    setupMappingPage();
    setupProfilePage();

    /* Select the top-most "None" item */
    m_mapTree->setCurrentItem(m_mapTree->topLevelItem(0));

    /* Listen to plugin configuration changes */
    connect(_app->inputMap(),
            SIGNAL(pluginConfigurationChanged(const QString&)),
            this, SLOT(slotPluginConfigurationChanged(const QString&)));

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

InputPatchEditor::~InputPatchEditor()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void InputPatchEditor::reject()
{
    _app->inputMap()->setPatch(m_universe, m_originalPluginName,
                               m_originalInput, m_originalFeedbackEnabled,
                               m_originalProfileName);

    QDialog::reject();
}

void InputPatchEditor::accept()
{
    if (m_editorUniverseRadio->isChecked() == true)
        _app->inputMap()->setEditorUniverse(m_universe);

    QDialog::accept();
}

/****************************************************************************
 * Mapping page
 ****************************************************************************/

void InputPatchEditor::setupMappingPage()
{
    /* Fill the map tree with available plugins */
    fillMappingTree();

    /* Selection changes */
    connect(m_mapTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,
                              QTreeWidgetItem*)),
            this, SLOT(slotMapCurrentItemChanged(QTreeWidgetItem*)));

    /* Configure button */
    connect(m_configureButton, SIGNAL(clicked()),
            this, SLOT(slotConfigureInputClicked()));

    /* Prevent the editor uni radio button from being unchecked manually */
    QButtonGroup* group = new QButtonGroup(this);
    group->addButton(m_editorUniverseRadio);

    m_feedbackEnabledCheck->setChecked(m_currentFeedbackEnabled);

    /* Set checked if the current universe is also the editor universe */
    if (_app->inputMap()->editorUniverse() == m_universe)
        m_editorUniverseRadio->setChecked(true);
}

void InputPatchEditor::fillMappingTree()
{
    /* Disable check state change tracking when the tree is filled */
    disconnect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
               this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));

    m_mapTree->clear();

    /* Add an empty item so that user can choose not to assign any plugin
       to an input universe */
    QTreeWidgetItem* pitem = new QTreeWidgetItem(m_mapTree);
    pitem->setText(KMapColumnName, KInputNone);
    pitem->setText(KMapColumnInput, QString("%1").arg(KInputInvalid));
    pitem->setFlags(pitem->flags() | Qt::ItemIsUserCheckable);

    /* Set "Nothing" selected if there is no valid input selected */
    if (m_currentInput == KInputInvalid)
        pitem->setCheckState(KMapColumnName, Qt::Checked);
    else
        pitem->setCheckState(KMapColumnName, Qt::Unchecked);

    /* Go thru available plugins and put them as the tree's root nodes. */
    QStringListIterator pit(_app->inputMap()->pluginNames());
    while (pit.hasNext() == true)
        fillPluginItem(pit.next(), new QTreeWidgetItem(m_mapTree));

    /* Enable check state change tracking after the tree has been filled */
    connect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));
}

void InputPatchEditor::fillPluginItem(const QString& pluginName, QTreeWidgetItem* pitem)
{
    QTreeWidgetItem* iitem = NULL;

    Q_ASSERT(pitem != NULL);

    /* Get rid of any existing children */
    while (pitem->childCount() > 0)
        delete pitem->child(0);

    pitem->setText(KMapColumnName, pluginName);
    pitem->setText(KMapColumnInput, QString("%1").arg(KInputInvalid));

    /* Go thru available inputs provided by each plugin and put
       them as their parent plugin's leaf nodes. */
    quint32 i = 0;
    QStringListIterator iit(_app->inputMap()->pluginInputs(pluginName));
    while (iit.hasNext() == true)
    {
        iitem = new QTreeWidgetItem(pitem);
        iitem->setText(KMapColumnName, iit.next());
        iitem->setText(KMapColumnInput, QString("%1").arg(i));
        iitem->setFlags(iitem->flags() | Qt::ItemIsUserCheckable);

        /* Select the currently mapped plugin input and expand
           its parent node. */
        if (m_currentPluginName == pluginName && m_currentInput == i)
        {
            iitem->setCheckState(KMapColumnName, Qt::Checked);
            pitem->setExpanded(true);
        }
        else
        {
            iitem->setCheckState(KMapColumnName, Qt::Unchecked);
            quint32 uni = _app->inputMap()->mapping(pluginName, i);
            if (uni != InputMap::invalidUniverse())
            {
                /* If a mapping exists for this plugin and
                   output, make it impossible to map it to
                   another universe. */
                iitem->setFlags(iitem->flags() & (!Qt::ItemIsEnabled));
                QString name = iitem->text(KMapColumnName);
                name += QString(" (Mapped to universe %1)").arg(uni + 1);
                iitem->setText(KMapColumnName, name);
            }
        }

        i++;
    }

    /* If no inputs were appended to the plugin node, put a
       "Nothing" node there. */
    if (i == 0)
    {
        iitem = new QTreeWidgetItem(pitem);
        iitem->setText(KMapColumnName, KInputNone);
        iitem->setText(KMapColumnInput, QString("%1").arg(KInputInvalid));
        iitem->setFlags(iitem->flags() & ~Qt::ItemIsEnabled);
        iitem->setFlags(iitem->flags() & ~Qt::ItemIsSelectable);
        iitem->setCheckState(KMapColumnName, Qt::Unchecked);
    }
}

void InputPatchEditor::slotMapCurrentItemChanged(QTreeWidgetItem* item)
{
    QString info;
    bool configurable;

    if (item == NULL)
    {
        info = _app->inputMap()->pluginStatus(QString(), 0);
        configurable = false;
    }
    else
    {
        QString plugin;
        quint32 input;

        if (item->parent() != NULL)
        {
            /* Input node selected */
            plugin = item->parent()->text(KMapColumnName);
            input = item->text(KMapColumnInput).toInt();
        }
        else
        {
            /* Plugin node selected */
            plugin = item->text(KMapColumnName);
            input = KInputInvalid;
        }

        info = _app->inputMap()->pluginStatus(plugin, input);
        configurable = _app->inputMap()->canConfigurePlugin(plugin);
    }

    /* Display information for the selected plugin or input */
    m_infoBrowser->setText(info);

    /* Enable configuration if plugin supports it */
    m_configureButton->setEnabled(configurable);
}

void InputPatchEditor::slotMapItemChanged(QTreeWidgetItem* item)
{
    if (item == NULL)
        return;

    if (item->checkState(KMapColumnName) == Qt::Checked)
    {
        /* Temporarily disable this signal to prevent an endless loop */
        disconnect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                   this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));

        /* Set all other items unchecked... */
        QTreeWidgetItemIterator it(m_mapTree);
        while ((*it) != NULL)
        {
            /* Don't touch the item that was just checked nor
               any parent nodes. */
            if (*it != item && (*it)->childCount() == 0)
            {
                /* Set all the rest of the nodes unchecked */
                (*it)->setCheckState(KMapColumnName,
                                     Qt::Unchecked);
            }

            /* Next one */
            ++it;
        }

        /* Start listening to this signal once again */
        connect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));
    }
    else
    {
        /* Don't allow unchecking an item by clicking it. Only allow
           changing the check state by checking another item. */
        item->setCheckState(KMapColumnName, Qt::Checked);
    }

    /* Store the selected plugin name & input */
    if (item->parent() != NULL)
    {
        m_currentPluginName = item->parent()->text(KMapColumnName);
        m_currentInput = item->text(KMapColumnInput).toInt();
    }
    else
    {
        m_currentPluginName = KInputNone;
        m_currentInput = KInputInvalid;
    }

    /* Apply the patch immediately so that input data can be used in the
       input profile editor */
    _app->inputMap()->setPatch(m_universe, m_currentPluginName,
                               m_currentInput, m_currentFeedbackEnabled,
                               m_currentProfileName);
}

void InputPatchEditor::slotConfigureInputClicked()
{
    QTreeWidgetItem* item;
    QString plugin;

    /* Find out the currently selected plugin */
    item = m_mapTree->currentItem();
    if (item == NULL)
        return;
    else if (item->parent() != NULL)
        plugin = item->parent()->text(KMapColumnName);
    else
        plugin = item->text(KMapColumnName);

    /* Configure the plugin */
    _app->inputMap()->configurePlugin(plugin);

    /* Refill the mapping tree in case configuration changed something */
    fillMappingTree();
}

void InputPatchEditor::slotFeedbackToggled(bool enable)
{
    m_currentFeedbackEnabled = enable;

    /* Apply the patch immediately so that input data can be used in the
       input profile editor */
    _app->inputMap()->setPatch(m_universe, m_currentPluginName,
                               m_currentInput, m_currentFeedbackEnabled,
                               m_currentProfileName);
}

void InputPatchEditor::slotPluginConfigurationChanged(const QString& pluginName)
{
    QTreeWidgetItem* item = pluginItem(pluginName);
    if (item == NULL)
        return;

    /* Disable check state tracking while the item is being filled */
    disconnect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
               this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));

    /* Re-fill the children for the plugin that's been changed */
    fillPluginItem(pluginName, pluginItem(pluginName));

    /* Enable check state tracking after the item has been filled */
    connect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));
}

QTreeWidgetItem* InputPatchEditor::pluginItem(const QString& pluginName)
{
    for (int i = 0; i < m_mapTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_mapTree->topLevelItem(i);
        if (item->text(KMapColumnName) == pluginName)
            return item;
    }

    return NULL;
}

/****************************************************************************
 * Profile tree
 ****************************************************************************/

void InputPatchEditor::setupProfilePage()
{
    connect(m_addProfileButton, SIGNAL(clicked()),
            this, SLOT(slotAddProfileClicked()));
    connect(m_removeProfileButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveProfileClicked()));
    connect(m_editProfileButton, SIGNAL(clicked()),
            this, SLOT(slotEditProfileClicked()));

    /* Fill the profile tree with available profile names */
    fillProfileTree();

    /* Listen to itemChanged() signals to catch check state changes */
    connect(m_profileTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotProfileItemChanged(QTreeWidgetItem*)));

    /* Double click acts as edit button click */
    connect(m_profileTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotEditProfileClicked()));
}

void InputPatchEditor::fillProfileTree()
{
    QTreeWidgetItem* item;

    m_profileTree->clear();

    /* Add an option for having no profile at all */
    item = new QTreeWidgetItem(m_profileTree);
    updateProfileItem(KInputNone, item);

    /* Insert available input profiles to the tree */
    QStringListIterator it(_app->inputMap()->profileNames());
    while (it.hasNext() == true)
    {
        item = new QTreeWidgetItem(m_profileTree);
        updateProfileItem(it.next(), item);
    }
}

void InputPatchEditor::updateProfileItem(const QString& name,
        QTreeWidgetItem* item)
{
    Q_ASSERT(item != NULL);

    item->setText(KProfileColumnName, name);

    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    if (m_currentProfileName == name)
        item->setCheckState(KProfileColumnName, Qt::Checked);
    else
        item->setCheckState(KProfileColumnName, Qt::Unchecked);
}

void InputPatchEditor::slotProfileItemChanged(QTreeWidgetItem* item)
{
    if (item->checkState(KProfileColumnName) == Qt::Checked)
    {
        /* Temporarily disable this signal to prevent an endless loop */
        disconnect(m_profileTree,
                   SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                   this,
                   SLOT(slotProfileItemChanged(QTreeWidgetItem*)));

        /* Set all other items unchecked... */
        QTreeWidgetItemIterator it(m_profileTree);
        while (*it != NULL)
        {
            /* ...except the one that was just checked */
            if (*it != item)
                (*it)->setCheckState(KProfileColumnName,
                                     Qt::Unchecked);
            ++it;
        }

        /* Start listening to this signal once again */
        connect(m_profileTree,
                SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                this,
                SLOT(slotProfileItemChanged(QTreeWidgetItem*)));
    }
    else
    {
        /* Don't allow unchecking an item by clicking it. Only allow
           changing the check state by checking another item. */
        item->setCheckState(KProfileColumnName, Qt::Checked);
    }

    /* Store the selected profile name */
    m_currentProfileName = item->text(KProfileColumnName);

    /* Apply the patch immediately */
    _app->inputMap()->setPatch(m_universe, m_currentPluginName,
                               m_currentInput, m_currentFeedbackEnabled,
                               m_currentProfileName);
}

void InputPatchEditor::slotAddProfileClicked()
{
    /* Create a new input profile and start editing it */
    InputProfileEditor ite(this, NULL);
edit:
    if (ite.exec() == QDialog::Accepted)
    {
        QLCInputProfile* profile;
        QString path;
        QDir dir;
        QString manufacturer;
        QString model;

        /* Remove spaces from these */
        manufacturer = ite.profile()->manufacturer().remove(QChar(' '));
        model = ite.profile()->model().remove(QChar(' '));

#ifdef Q_WS_X11
        /* If the current user is root, use the system profile dir
           for saving profiles. Otherwise use the user's home dir.
           This is done on Linux only, because Win32 & OSX save
           system profiles in a user-writable directory. */
        if (geteuid() == 0)
        {
            dir = QDir(INPUTPROFILEDIR);
        }
        else
        {
            path = QString("%1/%2").arg(getenv("HOME"))
                   .arg(USERINPUTPROFILEDIR);
            dir = QDir(path);
        }

        /* Ensure that the selected profile directory exists */
        if (dir.exists() == false)
            dir.mkpath(".");

        QMessageBox::information(this, "x11", dir.path());
#else
#ifdef __APPLE__
        /* Use the app bundle input profile dir for OSX */
        dir = QDir(QString("%1/../%2").arg(QApplication::applicationDirPath())
                   .arg(INPUTPROFILEDIR));
#else
        /* Use the system input profile dir for Windows */
        dir = QDir(INPUTPROFILEDIR);
#endif
#endif
        QMessageBox::information(this, "", dir.path());

        /* Construct a descriptive file name for the profile */
        path = QString("%1/%2-%3%4").arg(dir.absolutePath())
               .arg(manufacturer).arg(model)
               .arg(KExtInputProfile);

        /* Ensure that creating a new input profile won't overwrite
           an existing file. */
        if (QFile::exists(path + KExtInputProfile) == true)
        {
            for (int i = 1; i < INT_MAX; i++)
            {
                /* Start adding a number suffix to the file
                   name and stop when a unique file name is
                   found. */
                path = QString("%1/%2-%3-%4%5")
                       .arg(dir.absolutePath())
                       .arg(manufacturer).arg(model)
                       .arg(i).arg(KExtInputProfile);

                if (QFile::exists(path) == false)
                    break;
            }
        }

        /* Create a new non-const copy of the profile and
           reparent it to the input map */
        profile = new QLCInputProfile(*ite.profile());

        /* Save it to a file, go back to edit if save failed */
        if (profile->saveXML(path) == false)
        {
            QMessageBox::warning(this, tr("Saving failed"),
                                 tr("Unable to save the profile to %1")
                                 .arg(QDir::toNativeSeparators(path)));
            delete profile;
            goto edit;
        }
        else
        {
            /* Add the new profile to input map */
            _app->inputMap()->addProfile(profile);

            /* Add the new profile to our tree widget */
            QTreeWidgetItem* item;
            item = new QTreeWidgetItem(m_profileTree);
            updateProfileItem(profile->name(), item);
        }
    }
}

void InputPatchEditor::slotRemoveProfileClicked()
{
    QLCInputProfile* profile;
    QTreeWidgetItem* item;
    QString name;
    int r;

    /* Find out the currently selected item */
    item = m_profileTree->currentItem();
    if (item == NULL)
        return;

    /* Get the currently selected profile object by its name */
    name = item->text(KProfileColumnName);
    profile = _app->inputMap()->profile(name);
    if (profile == NULL)
        return;

    /* Ask for user confirmation */
    r = QMessageBox::question(this, tr("Delete profile"),
                              tr("Do you wish to permanently delete profile \"%1\"?")
                              .arg(profile->name()),
                              QMessageBox::Yes, QMessageBox::No);
    if (r == QMessageBox::Yes)
    {
        /* Attempt to delete the file first */
        QFile file(profile->path());
        if (file.remove() == true)
        {
            if (item->checkState(KProfileColumnName) == Qt::Checked)
            {
                /* The currently assigned profile is removed,
                   so select "None" next. */
                QTreeWidgetItem* none;
                none = m_profileTree->topLevelItem(0);
                Q_ASSERT(none != NULL);
                none->setCheckState(KProfileColumnName,
                                    Qt::Checked);
            }

            /* Successful deletion. Remove the profile from
               input map and our tree widget */
            _app->inputMap()->removeProfile(name);
            delete item;
        }
        else
        {
            /* Annoy the user even more after deletion failure */
            QMessageBox::warning(this, tr("File deletion failed"),
                                 tr("Unable to delete file %1")
                                 .arg(file.errorString()));
        }
    }
}

void InputPatchEditor::slotEditProfileClicked()
{
    QLCInputProfile* profile;
    QTreeWidgetItem* item;
    QString name;

    /* Get the currently selected item and bail out if nothing or "None"
       is selected */
    item = m_profileTree->currentItem();
    if (item == NULL || item->text(KProfileColumnName) == KInputNone)
        return;

    /* Get the currently selected profile by its name */
    name = item->text(KProfileColumnName);
    profile = _app->inputMap()->profile(name);
    if (profile == NULL)
        return;

    /* Edit the profile and update the item if OK was pressed */
    InputProfileEditor ite(this, profile);
edit:
    if (ite.exec() == QDialog::Rejected)
        return;

    QString path;

    /* Copy the channel's contents from the editor's copy to
       the actual object (with QLCInputProfile::operator=()). */
    *profile = *ite.profile();

#ifdef Q_WS_X11
    /* If the current user is root, save the profile to its old path.
       Otherwise use the user's home dir and generate a new file name
       if necessary. This is done on Linux only, because Win32 & OSX save
       profiles always in a user-writable directory. */
    if (geteuid() == 0)
    {
        path = profile->path();
    }
    else
    {
        QString manufacturer;
        QString model;

        /* Remove spaces from these */
        manufacturer = profile->manufacturer().remove(QChar(' '));
        model = profile->model().remove(QChar(' '));

        /* Ensure that user profile directory exists */
        path = QString("%1/%2").arg(getenv("HOME"))
               .arg(USERINPUTPROFILEDIR);
        QDir dir = QDir(path);
        if (dir.exists() == false)
            dir.mkpath(".");

        /* Check, whether the profile was originally saved
           in the system directory. If it is, construct a
           new name for it into the user's profile dir. */
        path = profile->path();
        if (path.contains(getenv("HOME")) == false)
        {
            /* Construct a descriptive file name for
               the profile under user's HOME dir */
            path = QString("%1/%2-%3%4").arg(dir.absolutePath())
                   .arg(manufacturer).arg(model)
                   .arg(KExtInputProfile);
        }
    }
#else
    /* Win32 & OSX save input profiles in a user-writable directory,
       so we can use that directly. */
    path = profile->path();
#endif
    /* Save the profile */
    if (profile->saveXML(path) == true)
    {
        /* Get the profile's name from the profile itself
           since it may have changed making local variable
           "name" invalid */
        updateProfileItem(profile->name(), item);
    }
    else
    {
        QMessageBox::warning(this, tr("Saving failed"),
                             tr("Unable to save %1 to %2")
                             .arg(profile->name())
                             .arg(QDir::toNativeSeparators(path)));
        goto edit;
    }
}
