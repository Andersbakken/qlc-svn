/*
  Q Light Controller
  breakoutwindow.cpp

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

#include "breakoutwindow.h"
#include "app.h"
#include <QMenu>

BreakOutWindow::BreakOutWindow(QWidget *w, QMdiSubWindow *s) {
	widget = w;
	sub = s;
	broken = false;
	boFrame = new QFrame();
	// TODO: set up the context menu for the boFrame so break the window back in

	menuAction = s->systemMenu()->addAction(QString("&Break out"));
	menuAction->setShortcut(tr("Ctrl+B"));
	connect(menuAction, SIGNAL(triggered()), this, SLOT(slotBreakOutWindow()));
	connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(slotWindowDestroyed(QObject*)));
}

void BreakOutWindow::slotBreakOutWindow() {
	if (broken) {
		widget->setParent(sub);
		widget->show();
		widget->setFocus();
		sub->show();
		menuAction->setText(QString("&Break out"));
		menuAction->setStatusTip(tr("Separate the window from the main application"));
	} else {
		widget->setParent(boFrame);
		widget->show();
		widget->setFocus();
		sub->hide();
		boFrame->show();
		boFrame->setFocus();
		menuAction->setText(QString("&Break in"));
		menuAction->setStatusTip(tr("Join the window back to the main application"));
	}
	broken = !broken;
}

void BreakOutWindow::slotWindowDestroyed(QObject *) {
	delete this;
}
