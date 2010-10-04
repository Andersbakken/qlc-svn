/*
  Q Light Controller
  configuremidiout.h

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

#ifndef CONFIGUREMIDIOUT_H
#define CONFIGUREMIDIOUT_H

#include <QDialog>
#include "ui_configuremidiout.h"

class MIDIDevice;
class MIDIOut;

class ConfigureMIDIOut : public QDialog, public Ui_ConfigureMIDIOut
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ConfigureMIDIOut(QWidget* parent, MIDIOut* plugin);
    virtual ~ConfigureMIDIOut();

protected:
    MIDIOut* m_plugin;

    /*********************************************************************
     * List of devices
     *********************************************************************/
protected slots:
    /** Invoke refresh for the interface list */
    void slotRefreshClicked();

    /** Open an editor dialog for the selected MIDI device */
    void slotEditClicked();

    /** Callback for MIDIOut::deviceAdded() signals */
    void slotDeviceAdded(MIDIDevice* device);

    /** Callback for MIDIOut::deviceRemoved() signals */
    void slotDeviceRemoved(MIDIDevice* device);

protected:
    /** Refresh the interface list */
    void refreshList();
};

#endif
