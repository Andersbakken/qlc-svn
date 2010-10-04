/*
  Q Light Controller
  inputpatch.h

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

#ifndef INPUTPATCH_H
#define INPUTPATCH_H

#include <QObject>

#include "qlcinputprofile.h"
#include "qlctypes.h"

class InputPatchEditor;
class InputMapEditor;
class QLCInPlugin;
class InputMap;

class QDomDocument;
class QDomElement;

#define KXMLQLCInputPatchProfile "Profile"
#define KXMLQLCInputPatchUniverse "Universe"
#define KXMLQLCInputPatchPluginNone "None"
#define KXMLQLCInputPatchPlugin "Plugin"
#define KXMLQLCInputPatchInput "Input"
#define KXMLQLCInputPatch "Patch"

class InputPatch : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(InputPatch);

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    InputPatch(QObject* parent);
    virtual ~InputPatch();

    /********************************************************************
     * Properties
     ********************************************************************/
public:
    void set(QLCInPlugin* plugin, quint32 input, bool enableFeedback,
             QLCInputProfile* profile);

    QLCInPlugin* plugin() const {
        return m_plugin;
    }
    QString pluginName() const;

    quint32 input() const;
    QString inputName() const;

    QLCInputProfile* profile() const {
        return m_profile;
    }
    QString profileName() const;

    bool feedbackEnabled() const {
        return m_feedbackEnabled;
    }

protected:
    QLCInPlugin* m_plugin;
    quint32 m_input;
    QLCInputProfile* m_profile;
    bool m_feedbackEnabled;
};

#endif
