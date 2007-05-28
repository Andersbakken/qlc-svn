/*
  Q Light Controller - Fixture Editor
  editmode.h

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

#ifndef EDITMODE_H
#define EDITMODE_H

#include "uic_editmode.h"

class QLCFixtureMode;
class QLCFixture;
class QLCChannel;

class EditMode : public UI_EditMode
{
	Q_OBJECT
public:
	/** Use this constructor to edit an existing mode */
	EditMode(QWidget* parent, QLCFixtureMode* mode);

	/** Use this constructor to create a new mode for the fixture */
	EditMode(QWidget* parent, QLCFixture* fixture);

	/** Destructor */
	~EditMode();

	/** Initialize the dialog */
	void init();

	/** Get the mode that was being edited. Don't save the pointer! */
	QLCFixtureMode* mode() { return m_mode; }

public slots:
	void slotModeNameChanged(const QString& text);
	void slotAddChannelClicked();
	void slotRemoveChannelClicked();
	void slotRaiseChannelClicked();
	void slotLowerChannelClicked();

	void slotBulbTypeChanged(const QString &type);
	void slotBulbLumensChanged(int lumens);
	void slotBulbColourTemperatureChanged(const QString &kelvins);

	void slotWeightChanged(int weight);
	void slotWidthChanged(int width);
	void slotHeightChanged(int height);
	void slotDepthChanged(int depth);
	
	void slotLensNameChanged(const QString &name);
	void slotLensDegreesMinChanged(int degrees);
	void slotLensDegreesMaxChanged(int degrees);

	void slotFocusTypeChanged(const QString &type);
	void slotFocusPanMaxChanged(int degrees);
	void slotFocusTiltMaxChanged(int degrees);

protected:
	void refreshChannelList();
	QLCChannel* currentChannel();
	void selectChannel(const QString &name);

private:
	QLCFixtureMode* m_mode;
};

#endif
