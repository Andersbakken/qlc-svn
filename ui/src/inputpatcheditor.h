/*
  Q Light Controller
  inputpatcheditor.h

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

#ifndef INPUTPATCHEDITOR_H
#define INPUTPATCHEDITOR_H

#include <QDialog>

#include "qlcinputprofile.h"
#include "qlctypes.h"

#include "ui_inputpatcheditor.h"
#include "inputpatch.h"

class QStringList;
class InputPatch;
class InputMap;

class InputPatchEditor : public QDialog, public Ui_InputPatchEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(InputPatchEditor)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    InputPatchEditor(QWidget* parent, quint32 universe, const InputPatch* patch);
    ~InputPatchEditor();

protected slots:
    void reject();
    void accept();

protected:
    /** The input universe that is being edited */
    quint32 m_universe;

    QString m_originalPluginName;
    QString m_currentPluginName;

    quint32 m_originalInput;
    quint32 m_currentInput;

    QString m_originalProfileName;
    QString m_currentProfileName;

    bool m_originalFeedbackEnabled;
    bool m_currentFeedbackEnabled;

    /************************************************************************
     * Mapping page
     ************************************************************************/
protected:
    QTreeWidgetItem* currentlyMappedItem() const;
    void setupMappingPage();
    void fillMappingTree();
    void fillPluginItem(const QString& pluginName, QTreeWidgetItem* item);
    QTreeWidgetItem* pluginItem(const QString& pluginName);

protected slots:
    void slotMapCurrentItemChanged(QTreeWidgetItem* item);
    void slotMapItemChanged(QTreeWidgetItem* item);
    void slotConfigureInputClicked();
    void slotReconnectClicked();
    void slotFeedbackToggled(bool enable);
    void slotPluginConfigurationChanged(const QString& pluginName);

    /************************************************************************
     * Profile page
     ************************************************************************/
protected:
    void setupProfilePage();
    void fillProfileTree();
    void updateProfileItem(const QString& name, QTreeWidgetItem* item);
    QString fullProfilePath(const QString& manufacturer, const QString& model) const;

protected slots:
    void slotProfileItemChanged(QTreeWidgetItem* item);
    void slotAddProfileClicked();
    void slotRemoveProfileClicked();
    void slotEditProfileClicked();
};

#endif /* INPUTPATCHEDITOR_H */
