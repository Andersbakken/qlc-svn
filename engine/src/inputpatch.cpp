/*
  Q Light Controller
  inputpatch.cpp

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

#include <QObject>
#include <QDebug>
#include <QtXml>

#include "qlcinplugin.h"
#include "qlctypes.h"

#include "inputpatch.h"
#include "inputmap.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

InputPatch::InputPatch(QObject* parent) : QObject(parent)
{
    Q_ASSERT(parent != NULL);

    m_plugin = NULL;
    m_input = KInputInvalid;
    m_profile = NULL;
    m_feedbackEnabled = true;
}

InputPatch::~InputPatch()
{
    if (m_plugin != NULL)
        m_plugin->close(m_input);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void InputPatch::set(QLCInPlugin* plugin, quint32 input, bool enableFeedback,
                     QLCInputProfile* profile)
{
    if (m_plugin != NULL && m_input != KInputInvalid)
        m_plugin->close(m_input);

    m_plugin = plugin;
    m_input = input;
    m_profile = profile;
    m_feedbackEnabled = enableFeedback;

    /* Open the assigned plugin input */
    if (m_plugin != NULL && m_input != KInputInvalid)
        m_plugin->open(m_input);
}

QLCInPlugin* InputPatch::plugin() const
{
    return m_plugin;
}

QString InputPatch::pluginName() const
{
    if (m_plugin != NULL)
        return m_plugin->name();
    else
        return KInputNone;
}

quint32 InputPatch::input() const
{
    if (m_plugin != NULL && m_input < quint32(m_plugin->inputs().count()))
        return m_input;
    else
        return KInputInvalid;
}

QString InputPatch::inputName() const
{
    if (m_plugin != NULL && m_input != KInputInvalid &&
            m_input < quint32(m_plugin->inputs().count()))
        return m_plugin->inputs()[m_input];
    else
        return KInputNone;
}

QLCInputProfile* InputPatch::profile() const
{
    return m_profile;
}

QString InputPatch::profileName() const
{
    if (m_profile != NULL)
        return m_profile->name();
    else
        return KInputNone;
}

bool InputPatch::feedbackEnabled() const
{
    return m_feedbackEnabled;
}

