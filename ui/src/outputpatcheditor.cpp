/*
  Q Light Controller
  outputpatcheditor.cpp

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
#include <QTreeWidget>
#include <QToolButton>
#include <QSettings>
#include <QGroupBox>
#include <QVariant>
#include <QDebug>

#include "outputpatcheditor.h"
#include "fixturemanager.h"
#include "outputpatch.h"
#include "outputmap.h"
#include "monitor.h"
#include "apputil.h"
#include "app.h"

#define KColumnName   0
#define KColumnOutput 1
#define SETTINGS_GEOMETRY "outputpatcheditor/geometry"

extern App* _app;

OutputPatchEditor::OutputPatchEditor(QWidget* parent, quint32 universe,
                                     const OutputPatch* patch)
        : QDialog(parent)
{
    QSettings settings;
    QVariant value;
    QString key;

    setupUi(this);
    m_infoBrowser->setOpenExternalLinks(true);

    Q_ASSERT(universe < _app->outputMap()->universes());
    m_universe = universe;
    setWindowTitle(tr("Mapping properties for output universe %1")
                   .arg(universe + 1));

    m_originalPluginName = patch->pluginName();
    m_currentPluginName = patch->pluginName();

    m_originalOutput = patch->output();
    m_currentOutput = patch->output();

    /* Selection changes */
    connect(m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(slotCurrentItemChanged(QTreeWidgetItem*)));

    /* Configure button */
    connect(m_configureButton, SIGNAL(clicked()),
            this, SLOT(slotConfigureClicked()));

    /* Reconnect button */
    connect(m_reconnectButton, SIGNAL(clicked()),
            this, SLOT(slotReconnectClicked()));

    /* Zero-based DMX setting */
    connect(m_zeroBasedDMXCheckBox, SIGNAL(clicked()),
            this, SLOT(slotZeroBasedDMXClicked()));
    m_zeroBasedDMXCheckBox->setChecked(patch->isDMXZeroBased());
    m_originalDMXZeroBasedSetting = m_zeroBasedDMXCheckBox->isChecked();

    fillTree();

    /* Listen to plugin configuration changes */
    connect(_app->outputMap(),
            SIGNAL(pluginConfigurationChanged(const QString&)),
            this, SLOT(slotPluginConfigurationChanged(const QString&)));

    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

OutputPatchEditor::~OutputPatchEditor()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void OutputPatchEditor::reject()
{
    /* Revert changes to original values (stored when this dialog opens) */
    _app->outputMap()->setPatch(m_universe, m_originalPluginName,
                                m_originalOutput);

    storeDMXZeroBasedSetting(m_originalDMXZeroBasedSetting);

    QDialog::reject();
}

QTreeWidgetItem* OutputPatchEditor::currentlyMappedItem() const
{
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* pluginItem = m_tree->topLevelItem(i);
        Q_ASSERT(pluginItem != NULL);

        if (pluginItem->text(KColumnName) == m_currentPluginName)
        {
            QTreeWidgetItem* outputItem = pluginItem->child(m_currentOutput);
            return outputItem;
        }
    }

    return NULL;
}

void OutputPatchEditor::fillTree()
{
    /* Disable check state change tracking when the tree is filled */
    disconnect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
               this, SLOT(slotItemChanged(QTreeWidgetItem*)));

    m_tree->clear();

    /* Add an empty item so that user can choose not to assign any plugin
       to an input universe */
    QTreeWidgetItem* pitem = new QTreeWidgetItem(m_tree);
    pitem->setText(KColumnName, KOutputNone);
    pitem->setText(KColumnOutput, QString::number(KOutputInvalid));
    pitem->setFlags(pitem->flags() | Qt::ItemIsUserCheckable);

    /* Set "Nothing" selected if there is no valid output selected */
    if (m_currentOutput == KOutputInvalid)
        pitem->setCheckState(KColumnName, Qt::Checked);
    else
        pitem->setCheckState(KColumnName, Qt::Unchecked);

    /* Go thru available plugins and put them as the tree's root nodes. */
    QStringListIterator pit(_app->outputMap()->pluginNames());
    while (pit.hasNext() == true)
        fillPluginItem(pit.next(), new QTreeWidgetItem(m_tree));

    /* Enable check state change tracking after the tree has been filled */
    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChanged(QTreeWidgetItem*)));
}

void OutputPatchEditor::fillPluginItem(const QString& pluginName,
                                       QTreeWidgetItem* pitem)
{
    Q_ASSERT(pitem != NULL);

    /* Get rid of any existing children */
    while (pitem->childCount() > 0)
        delete pitem->child(0);

    pitem->setText(KColumnName, pluginName);
    pitem->setText(KColumnOutput, QString::number(KOutputInvalid));

    /* Go thru available inputs provided by each plugin and put them as their
       parent plugin's leaf nodes */
    quint32 i = 0;
    QStringListIterator iit(_app->outputMap()->pluginOutputs(pluginName));
    while (iit.hasNext() == true)
    {
        QTreeWidgetItem* iitem = new QTreeWidgetItem(pitem);
        iitem->setText(KColumnName, iit.next());
        iitem->setText(KColumnOutput, QString::number(i));
        iitem->setFlags(iitem->flags() | Qt::ItemIsUserCheckable);

        /* Select the currently mapped output and expand its parent node */
        if (m_currentPluginName == pluginName && m_currentOutput == i)
        {
            iitem->setCheckState(KColumnName, Qt::Checked);
            pitem->setExpanded(true);
        }
        else
        {
            iitem->setCheckState(KColumnName, Qt::Unchecked);
            quint32 uni = _app->outputMap()->mapping(pluginName, i);
            if (uni != OutputMap::invalidUniverse())
            {
                /* If a mapping exists for this plugin and output, make it
                   impossible to map it to another universe. */
                iitem->setFlags(iitem->flags() & (!Qt::ItemIsEnabled));
                QString name = iitem->text(KColumnName);
                name += QString(" (Mapped to universe %1)").arg(uni + 1);
                iitem->setText(KColumnName, name);
            }
        }

        i++;
    }

    /* If no outputs were appended to the plugin node, put a "Nothing" node there */
    if (i == 0)
    {
        QTreeWidgetItem* iitem = new QTreeWidgetItem(pitem);
        iitem->setText(KColumnName, KOutputNone);
        iitem->setText(KColumnOutput, QString::number(KOutputInvalid));
        iitem->setFlags(iitem->flags() & ~Qt::ItemIsEnabled);
        iitem->setFlags(iitem->flags() & ~Qt::ItemIsSelectable);
        iitem->setCheckState(KColumnName, Qt::Unchecked);
    }
}

QTreeWidgetItem* OutputPatchEditor::pluginItem(const QString& pluginName)
{
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
        if (item->text(KColumnName) == pluginName)
            return item;
    }

    return NULL;
}

void OutputPatchEditor::slotCurrentItemChanged(QTreeWidgetItem* item)
{
    QString info;
    bool configurable;

    if (item == NULL)
    {
        /* Nothing selected */
        info = _app->outputMap()->pluginStatus(QString(), 0);
        configurable = false;
    }
    else
    {
        QString plugin;
        quint32 output;

        if (item->parent() != NULL)
        {
            /* Output node selected */
            plugin = item->parent()->text(KColumnName);
            output = item->text(KColumnOutput).toInt();
        }
        else
        {
            /* Plugin node selected */
            plugin = item->text(KColumnName);
            output = KOutputInvalid;
        }

        info = _app->outputMap()->pluginStatus(plugin, output);
        configurable = _app->outputMap()->canConfigurePlugin(plugin);
    }

    /* Display information for the selected plugin or output */
    m_infoBrowser->setText(info);

    /* Enable configuration if plugin supports it */
    m_configureButton->setEnabled(configurable);
}

void OutputPatchEditor::slotItemChanged(QTreeWidgetItem* item)
{
    if (item == NULL)
        return;

    if (item->checkState(KColumnName) == Qt::Checked)
    {
        /* Temporarily disable this signal to prevent an endless loop */
        disconnect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                   this, SLOT(slotItemChanged(QTreeWidgetItem*)));

        /* Set all other items unchecked... */
        QTreeWidgetItemIterator it(m_tree);
        while ((*it) != NULL)
        {
            /* Don't touch the item that was just checked nor
               any parent nodes. */
            if (*it != item && (*it)->childCount() == 0)
            {
                /* Set all the rest of the nodes unchecked */
                (*it)->setCheckState(KColumnName, Qt::Unchecked);
            }

            /* Next one */
            ++it;
        }

        /* Start listening to this signal once again */
        connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                this, SLOT(slotItemChanged(QTreeWidgetItem*)));
    }
    else
    {
        /* Don't allow unchecking an item by clicking it. Only allow
           changing the check state by checking another item. */
        item->setCheckState(KColumnName, Qt::Checked);
    }

    /* Store the selected plugin name & input */
    if (item->parent() != NULL)
    {
        m_currentPluginName = item->parent()->text(KColumnName);
        m_currentOutput = item->text(KColumnOutput).toInt();
    }
    else
    {
        m_currentPluginName = QString();
        m_currentOutput = KOutputInvalid;
    }

    /* Apply the patch immediately so that input data can be used in the
       input profile editor */
    _app->outputMap()->setPatch(m_universe, m_currentPluginName, m_currentOutput);
}

void OutputPatchEditor::slotPluginConfigurationChanged(const QString& pluginName)
{
    QTreeWidgetItem* item = pluginItem(pluginName);
    if (item == NULL)
        return;

    /* Disable check state tracking while the item is being filled */
    disconnect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
               this, SLOT(slotItemChanged(QTreeWidgetItem*)));

    /* Re-fill the children for the plugin that's been changed */
    fillPluginItem(pluginName, pluginItem(pluginName));
    slotCurrentItemChanged(item);

    /* Enable check state tracking after the item has been filled */
    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChanged(QTreeWidgetItem*)));
}

void OutputPatchEditor::slotConfigureClicked()
{
    QTreeWidgetItem* item;
    QString plugin;

    /* Find out the currently selected plugin */
    item = m_tree->currentItem();
    if (item == NULL)
        return;
    else if (item->parent() != NULL)
        plugin = item->parent()->text(KColumnName);
    else
        plugin = item->text(KColumnName);

    /* Configure the plugin. Changes in plugin outputs are handled with
       slotPluginConfigurationChanged(). */
    _app->outputMap()->configurePlugin(plugin);
}

void OutputPatchEditor::slotReconnectClicked()
{
    OutputPatch* outputPatch = _app->outputMap()->patch(m_universe);
    if (outputPatch != NULL)
        outputPatch->set(outputPatch->plugin(), outputPatch->output());

    QTreeWidgetItem* outputItem = currentlyMappedItem();
    if (outputItem != NULL)
    {
        if (outputItem->parent() != NULL)
            outputItem->parent()->setExpanded(true);
        m_tree->setCurrentItem(outputItem);
    }
}

void OutputPatchEditor::slotZeroBasedDMXClicked()
{
    storeDMXZeroBasedSetting(m_zeroBasedDMXCheckBox->isChecked());
}

void OutputPatchEditor::storeDMXZeroBasedSetting(bool set)
{
    OutputPatch* outputPatch = _app->outputMap()->patch(m_universe);
    if (outputPatch != NULL)
        outputPatch->setDMXZeroBased(set);

    /* Update fixture manager so the setting is visible immediately */
    if (FixtureManager::instance() != NULL)
        FixtureManager::instance()->updateView();

    /* Update monitor so the setting is visible immediately */
    if (Monitor::instance() != NULL)
        Monitor::instance()->updateFixtureLabelStyles();
}
