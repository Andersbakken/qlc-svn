/*
  Q Light Controller
  configureftdidmx.h
  
  Copyright (c) Christopher Staite
  
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

#ifndef CONFIGUREFTDIDMX_H
#define CONFIGUREFTDIDMX_H

#include <QDialog>
#include "common/qlctypes.h"

#include "ui_configureftdidmx.h"

class FTDIDMXOut;
class QTimer;

struct FTDIDevice {
	char *name;
	int vid;
	int pid;	
};
// Now struct can be used as QVarient
Q_DECLARE_METATYPE(FTDIDevice);

static struct FTDIDevice known_devices[] = {
	{"Homebrew USB -> DMX", 0x0403, 0xEC70},
	{"EntTec Open DMX USB", 0x0403, 0x6001},
	{"Other", 0x0000, 0x0000}
};

class ConfigureFTDIDMXOut : public QDialog, public Ui_ConfigureFTDIDMXOut
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	ConfigureFTDIDMXOut(QWidget* parent, FTDIDMXOut* plugin);
	virtual ~ConfigureFTDIDMXOut();

protected:
	FTDIDMXOut* m_plugin;

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
	t_output m_output;

	/*********************************************************************
	 * Refresh
	 *********************************************************************/
protected slots:
	/**
	 * Invoke refresh for the interface list
	 */
	void slotRefreshClicked();
	void slotDeviceChanged(int index);

protected:
	/** Refresh the interface list */
	void refreshList();
	int getIntHex(QLineEdit *e);

	int m_current_vid;
	int m_current_pid;
};

#endif
