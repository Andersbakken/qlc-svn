/*
  Q Light Controller
  configureudmxout.h

  Copyright (c)	Lutz Hillebrand
		Heikki Junnila

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
#ifndef CONFIGUREUDMXOUT_H
#define CONFIGUREUDMXOUT_H

#include <QDialog>
#include "common/qlctypes.h"

#include "ui_configureudmxout.h"

class UDMXOut;

class ConfigureUDMXOut : public QDialog, public Ui_ConfigureUDMXOut
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	ConfigureUDMXOut(QWidget* parent, UDMXOut* plugin);
	virtual ~ConfigureUDMXOut();

protected:
	UDMXOut* m_plugin;

	/*********************************************************************
	 * Universe testing
	 *********************************************************************/
protected slots:
	/** Start or stop flashing all channel values of one universe */
	void slotTestToggled(bool state);

	/** Flash all channels of one universe between 0 and 255 */
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
	/** Invoke refresh for the interface list */
	void slotRefreshClicked();

protected:
	/** Refresh the interface list */
	void refreshList();
};

#endif
