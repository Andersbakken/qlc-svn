/*
  Q Light Controller
  breakoutwindow.h
  
  Copyright (C) Christopher Staite
  
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

#ifndef BREAKOUTWINDOW_H
#define BREAKOUTWINDOW_H

#include <QWidget>
#include <QMdiSubWindow>
#include <QAction>
#include <QFrame>

class BreakOutWindow : public QObject {
	Q_OBJECT

public:
	BreakOutWindow(QWidget *w, QMdiSubWindow *s);

protected:
	QFrame *boFrame;
	QWidget *widget;
	QMdiSubWindow *sub;
	QAction *menuAction;
	bool broken;

protected slots:
	void slotBreakOutWindow();
	void slotWindowDestroyed(QObject *);

};

#endif
