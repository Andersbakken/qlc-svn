/*
  Q Light Controller
  qlcorkspace.h

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

#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <qworkspace.h>
#include <qevent.h>

class QLCWorkspace : public QWorkspace
{
	Q_OBJECT

 public:
	QLCWorkspace(QWidget* parent);
	~QLCWorkspace();

	/** Set the background image from the given file name */
	void setBackground(const QString& path);

	/** Set the background image with a file selection dialog */
	void setBackground();

 signals:
	void rightMouseButtonClicked(const QPoint& pos);
	void backgroundChanged(const QString& path);

 protected slots:
	void slotRightMouseButtonClicked(const QPoint& pos);
	void slotMenuCallback(int item);

 protected:
	void mousePressEvent(QMouseEvent* event);

 protected:
	QString m_backgroundPath;

};

#endif
