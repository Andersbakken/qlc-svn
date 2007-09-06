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

#include "uic_virtualconsoleproperties.h"

class VirtualConsoleProperties : public UI_VirtualConsoleProperties
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	VirtualConsoleProperties(QWidget* parent);
	~VirtualConsoleProperties();

	void init();

	/*********************************************************************
	 * Grid
	 *********************************************************************/
 public:
	void setGridEnabled(bool set) { m_gridEnabled = set; }
	bool isGridEnabled() { return m_gridEnabled; }

	void setGridX(int x) { m_gridX = x; }
	int gridX() { return m_gridX; }

	void setGridY(int y) { m_gridY = y; }
	int gridY() { return m_gridY; }

 protected slots:
	void slotGridEnabledCheckToggled(bool state);

 protected:
	bool m_gridEnabled;
	int m_gridX;
	int m_gridY;

	/*********************************************************************
	 * Key repeat
	 *********************************************************************/
 public:
	void setKeyRepeatOff(bool set) { m_keyRepeatOff = set; }
	bool isKeyRepeatOff() { return m_keyRepeatOff; }

 protected:
	bool m_keyRepeatOff;

	/*********************************************************************
	 * Key repeat
	 *********************************************************************/
 public:
	void setGrabKeyboard(bool set) { m_grabKeyboard = set; }
	bool isGrabKeyboard() { return m_grabKeyboard; }

 protected:
	bool m_grabKeyboard;

	/*********************************************************************
	 * OK/Cancel
	 *********************************************************************/
 protected slots:
	void slotOKClicked();
	void slotCancelClicked();
};

#endif
