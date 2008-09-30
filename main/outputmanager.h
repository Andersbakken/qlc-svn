/*
  Q Light Controller
  outputmanager.h
  
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

#ifndef OUTPUTMANAGER_H
#define OUTPUTMANAGER_H

#include <QWidget>

#include "ui_outputmanager.h"

class QTreeWidget;
class QPushButton;
class OutputMap;

class OutputManager : public QWidget, public Ui_OutputManager
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	OutputManager(QWidget* parent);
	virtual ~OutputManager();

	/*********************************************************************
	 * Tree widget
	 *********************************************************************/
protected slots:
	void slotEditClicked();
};

#endif
