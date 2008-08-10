/*
  Q Light Controller
  configureusbdmxout.h
  
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

#ifndef CONFIGUREUSBDMXOUT_H
#define CONFIGUREUSBDMXOUT_H

#include <QDialog>

#include "ui_configureusbdmxout.h"

class USBDMXOut;
class QTimer;

class ConfigureUSBDMXOut : public QDialog, public Ui_ConfigureUSBDMXOut
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	ConfigureUSBDMXOut(QWidget* parent, USBDMXOut* plugin);
	virtual ~ConfigureUSBDMXOut();

protected:
	USBDMXOut* m_plugin;

	/*********************************************************************
	 * Universe testing
	 *********************************************************************/
protected slots:
	/**
	 * Start/stop flashing all channel values of one universe
	 *
	 * @param state true to start flashing, false to stop flashing
	 */
	void slotTestToggled(bool state);

	/**
	 * Flash all channels of one universe between 0 and 255
	 */
	void slotTestTimeout();

protected:
	/** Timer that drives universe testing */
	QTimer* m_timer;

	/** Modulo var that changes state between [0|1] on each timer pass */
	int m_testMod;

	/** The universe to test output on */
	int m_testUniverse;

	/*********************************************************************
	 * Refresh
	 *********************************************************************/
protected slots:
	/**
	 * Invoke refresh for the interface list
	 */
	void slotRefreshClicked();

protected:
	/** Refresh the interface list */
	void refreshList();
};

#endif
