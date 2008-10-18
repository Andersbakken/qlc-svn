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
	void fillTree();
	void updateItem(QTreeWidgetItem* item, InputPatch* patch,
			t_input_universe universe);

protected:
	QTreeWidget* m_tree;
};

#endif
