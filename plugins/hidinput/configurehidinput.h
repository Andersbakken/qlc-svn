/*
  Q Light Controller
  configurehidinput.h

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

#ifndef CONFIGUREHIDINPUT_H
#define CONFIGUREHIDINPUT_H

#include "ui_configurehidinput.h"

class HIDInput;
class HIDDevice;

class ConfigureHIDInput : public QDialog, public Ui_ConfigureHIDInput
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ConfigureHIDInput(QWidget* parent, HIDInput* plugin);
    virtual ~ConfigureHIDInput();

protected:
    HIDInput* m_plugin;

    /*********************************************************************
     * Refresh
     *********************************************************************/
protected slots:
    /**
     * Invoke refresh for the interface list
     */
    void slotRefreshClicked();

    /**
     * Callback for HIDInput::deviceAdded() signals.
     */
    void slotDeviceAdded(HIDDevice* device);

    /**
     * Callback for HIDInput::deviceRemoved() signals.
     */
    void slotDeviceRemoved(HIDDevice* device);

protected:
    /** Refresh the interface list */
    void refreshList();
};

#endif
