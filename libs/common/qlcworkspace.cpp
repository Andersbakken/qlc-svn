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

#include <QMouseEvent>
#include <QFileDialog>
#include <iostream>
#include <QMenu>
#include <QIcon>
#include <QtXml>

#include "common/qlcworkspace.h"

QLCWorkspace::QLCWorkspace(QWidget* parent) : QWorkspace(parent)
{
	connect(this, SIGNAL(rightMouseButtonClicked(const QPoint&)),
		this, SLOT(slotRightMouseButtonClicked(const QPoint&)));
}

QLCWorkspace::~QLCWorkspace()
{
}

/*****************************************************************************
 * Background image
 *****************************************************************************/

QString QLCWorkspace::backgroundImage()
{
	return m_backgroundImage;
}

void QLCWorkspace::setBackgroundImage(const QString& path)
{
	std::cout << "Not implemented" << endl;
/*
	if (path.isEmpty() == false)
		setBackgroundPixmap(QPixmap(path));
	else
		setBackgroundPixmap(NULL);

	m_backgroundImage = path;
*/
	emit backgroundChanged(path);
}

void QLCWorkspace::slotSetBackgroundImage()
{
	std::cout << "Not implemented" << endl;
/*
	QString path;

	QLCImagePreview* preview = new QLCImagePreview;
  
	QFileDialog* fd = new QFileDialog( this );
	fd->setCaption("Choose a background image");
	fd->setContentsPreviewEnabled(true);
	fd->setContentsPreview(preview, preview);
	fd->setPreviewMode(QFileDialog::Contents);
	fd->setFilter("Images (*.png *.xpm *.jpg *.gif)");
	fd->setSelection(m_backgroundImage);
  
	if (fd->exec() == QDialog::Accepted)
	{
		path = fd->selectedFile();
    
		if (path.isEmpty() == false)
			setBackgroundImage(path);
	}

	delete preview;
	delete fd;
*/
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool QLCWorkspace::loadXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	if (wksp_root->tagName() != KXMLQLCWorkspace)
	{
		std::cout << "Workspace node not found!" << endl;
		return false;
	}

	/* Background image */
	str = wksp_root->attribute(KXMLQLCWorkspaceBackgroundImage);
	setBackgroundImage(str);

	return true;
}

bool QLCWorkspace::saveXML(QDomDocument*doc, QDomElement* wksp_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* Background image */
	wksp_root->setAttribute(KXMLQLCWorkspaceBackgroundImage,
				m_backgroundImage);

	return true;
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void QLCWorkspace::loadDefaults(const QString& path)
{
	QSettings settings(path, path);
	QString value;

	value = settings.value(path + "/workspace/background/").toString();
	if (value.length() > 0)
		setBackgroundImage(value);
}

void QLCWorkspace::saveDefaults(const QString& path)
{
	QSettings settings(path, path);
	settings.setValue(path + "/workspace/background/", backgroundImage());
}

/*****************************************************************************
 * Menu
 *****************************************************************************/

void QLCWorkspace::slotRightMouseButtonClicked(const QPoint& pos)
{
	QMenu menu(this);
	QAction bgAction(QIcon(PIXMAPS "/image.png"), "Background image", this);

	connect(&bgAction, SIGNAL(triggered(bool)),
		this, SLOT(slotSetBackground()));

	menu.setTitle("Workspace");
	menu.addAction(&bgAction);

	menu.exec(pos);
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void QLCWorkspace::mousePressEvent(QMouseEvent* event)
{
	if (event->button() & Qt::RightButton)
		emit rightMouseButtonClicked(mapToGlobal(event->pos()));
}
