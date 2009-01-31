/*
  Q Light Controller
  virtualconsoleproperties.h

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

#ifndef VIRTUALCONSOLEPROPERTIES_H
#define VIRTUALCONSOLEPROPERTIES_H

#include <QDialog>

#include "ui_virtualconsoleproperties.h"
#include "common/qlctypes.h"
#include "functionconsumer.h"

class VirtualConsoleProperties : public QDialog,
	public Ui_VirtualConsoleProperties
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	VirtualConsoleProperties(QWidget* parent);
	~VirtualConsoleProperties();

private:
	Q_DISABLE_COPY(VirtualConsoleProperties)

	/*********************************************************************
	 * Grid
	 *********************************************************************/
public:
	void setGrid(bool enabled, int x, int y);

	bool isGridEnabled();
	int gridX();
	int gridY();

	/*********************************************************************
	 * Key repeat
	 *********************************************************************/
public:
	void setKeyRepeatOff(bool set);
	bool isKeyRepeatOff();

	/*********************************************************************
	 * Grab keyboard
	 *********************************************************************/
public:
	void setGrabKeyboard(bool set);
	bool isGrabKeyboard();

	/*********************************************************************
	 * Slider value ranges
	 *********************************************************************/
public:
	void setFadeLimits(t_bus_value low, t_bus_value high);
	t_bus_value fadeLowLimit();
	t_bus_value fadeHighLimit();

	void setHoldLimits(t_bus_value low, t_bus_value high);
	t_bus_value holdLowLimit();
	t_bus_value holdHighLimit();

	/*********************************************************************
	 * Slider external input
	 *********************************************************************/
public:
	void setFadeInputSource(t_input_universe uni, t_input_channel ch);
	t_input_universe fadeInputUniverse();
	t_input_channel fadeInputChannel();

	void setHoldInputSource(t_input_universe uni, t_input_channel ch);
	t_input_universe holdInputUniverse();
	t_input_channel holdInputChannel();

protected:
	t_input_universe m_fadeInputUniverse;
	t_input_channel m_fadeInputChannel;

	t_input_universe m_holdInputUniverse;
	t_input_channel m_holdInputChannel;

protected slots:
	void slotChooseFadeInputClicked();
	void slotChooseHoldInputClicked();

protected:
	void updateFadeInputSource();
	void updateHoldInputSource();

	/*********************************************************************
	 * Timer type
	 *********************************************************************/
public:
	void setTimerType(FunctionConsumer::TimerType type);
	FunctionConsumer::TimerType timerType();
};

#endif
