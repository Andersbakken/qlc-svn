/*
  Q Light Controller
  qlcworkspace.h

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

#ifndef QLCWORKSPACE_H
#define QLCWORKSPACE_H

#include <QWorkspace>
#include <QEvent>

#define KXMLQLCWorkspace "Workspace"
#define KXMLQLCWorkspaceBackgroundImage "BackgroundImage"
#define KXMLQLCWorkspaceTheme "Theme"

class QDomDocument;
class QDomElement;

class QLCWorkspace : public QWorkspace
{
	Q_OBJECT

public:
	QLCWorkspace(QWidget* parent);
	~QLCWorkspace();

	/*********************************************************************
	 * Background image
	 *********************************************************************/
public:
	/** Get the background image file name */
	QString backgroundImage();

	/** Set the background image from the given file name */
	void setBackgroundImage(const QString& path);

protected slots:
	/** Set the background image using a file selection dialog */
	void slotSetBackgroundImage();

signals:
	void backgroundChanged(const QString& path);

protected:
	QString m_backgroundImage;
	
	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool saveXML(QDomDocument*doc, QDomElement* wksp_root);

	/*********************************************************************
	 * Defaults
	 *********************************************************************/
public:
	void loadDefaults(const QString& path);
	void saveDefaults(const QString& path);

	/*********************************************************************
	 * Menu
	 *********************************************************************/
signals:
	void rightMouseButtonClicked(const QPoint& pos);

protected slots:
	void slotRightMouseButtonClicked(const QPoint& pos);
	void slotMenuCallback(int item);

	/*********************************************************************
	 * Event handlers
	 *********************************************************************/
protected:
	void mousePressEvent(QMouseEvent* event);
};

#endif
