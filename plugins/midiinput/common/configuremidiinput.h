/*
  Q Light Controller
  configuremidiinput.h

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

#ifndef CONFIGUREMIDIINPUT_H
#define CONFIGUREMIDIINPUT_H

#include "ui_configuremidiinput.h"

class MIDIDevice;
class MIDIInput;

class ConfigureMIDIInput : public QDialog, public Ui_ConfigureMIDIInput
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ConfigureMIDIInput(QWidget* parent, MIDIInput* plugin);
    virtual ~ConfigureMIDIInput();

protected:
    /** The plugin that is being edited */
    MIDIInput* m_plugin;

    /*********************************************************************
     * Tree widget
     *********************************************************************/
protected slots:
    /** Callback for HIDInput::deviceAdded() signals. */
    void slotDeviceAdded(MIDIDevice* device);

    /** Callback for HIDInput::deviceRemoved() signals. */
    void slotDeviceRemoved(MIDIDevice* device);

protected:
    /** Refresh the interface list */
    void refreshList();

    /*********************************************************************
     * Edit
     *********************************************************************/
protected slots:
    /** Invoke refresh for the interface list */
    void slotRefreshClicked();

    /**
     * Open a dialog with which to select a feedback output line, MIDI
     * channel and control mode.
     */
    void slotEditClicked();
};

#endif
