/*
  Q Light Controller
  inputmanager.h

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

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <QWidget>
#include "common/qlctypes.h"

class QTreeWidgetItem;
class QTreeWidget;
class QToolBar;
class QTimer;

class InputPatch;
class InputMap;

class InputManager : public QWidget
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	InputManager(QWidget* parent);
	virtual ~InputManager();

	void update();

private:
	Q_DISABLE_COPY(InputManager)

protected:
	void setupUi();

	/*********************************************************************
	 * Toolbar
	 *********************************************************************/
protected slots:
	void slotEditClicked();

protected:
	QToolBar* m_toolbar;

	 /*********************************************************************
	 * Tree widget
	 *********************************************************************/
protected:
	void updateItem(QTreeWidgetItem* item, InputPatch* patch,
			t_input_universe universe);

protected slots:
	void slotInputValueChanged(t_input_universe universe,
				   t_input_channel channel,
				   t_input_value value);
	void slotTimerTimeout();

protected:
	QTreeWidget* m_tree;
	QTimer* m_timer;
};

#endif
