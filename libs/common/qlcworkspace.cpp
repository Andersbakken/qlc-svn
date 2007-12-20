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

#include <qapplication.h>
#include <qpopupmenu.h>
#include <qpixmap.h>
#include <qfiledialog.h>
#include <qstylefactory.h>
#include <qsettings.h>
#include <qdom.h>

#include "common/settings.h"
#include "common/qlcworkspace.h"
#include "common/qlcimagepreview.h"

#define KMenuItemTitle            0
#define KMenuItemChangeBackground 1000
#define KMenuItemChangeTheme      2000

QLCWorkspace::QLCWorkspace(QWidget* parent) : QWorkspace(parent, "Workspace")
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
	if (path.isEmpty() == false)
		setBackgroundPixmap(QPixmap(path));
	else
		setBackgroundPixmap(NULL);

	m_backgroundImage = path;
	
	emit backgroundChanged(path);
}

void QLCWorkspace::setBackgroundImage()
{
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
}

/*****************************************************************************
 * Theme
 *****************************************************************************/

QString QLCWorkspace::theme()
{
	return m_theme;
}

void QLCWorkspace::setTheme(const QString& theme)
{
	if (theme.isEmpty() == false)
		QApplication::setStyle(theme);
	m_theme = theme;

	emit themeChanged(theme);
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
		qWarning("Workspace node not found!");
		return false;
	}

	/* Background image */
	str = wksp_root->attribute(KXMLQLCWorkspaceBackgroundImage);
	setBackgroundImage(str);

	/* Theme */
	str = wksp_root->attribute(KXMLQLCWorkspaceTheme);
	setTheme(str);

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

	/* Theme */
	wksp_root->setAttribute(KXMLQLCWorkspaceTheme, m_theme);

	return true;
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void QLCWorkspace::loadDefaults(const QString& path)
{
	QSettings settings;
	QString key;
	QString value;

	settings.setPath("qlc.sf.net", "qlc");

	key = path + QString("/workspace/background/");
	value = settings.readEntry(key);
	if (value.length() > 0)
		setBackgroundImage(value);

	key = path + QString("/workspace/theme/");
	value = settings.readEntry(key);
	if (value.length() > 0)
		setTheme(value);
}

void QLCWorkspace::saveDefaults(const QString& path)
{
	QSettings settings;
	QString key;

	settings.setPath("qlc.sf.net", "qlc");

	key = path + QString("/workspace/background/");
	settings.writeEntry(key, backgroundImage());

	key = path + QString("/workspace/theme/");
	settings.writeEntry(key, theme());
}

/*****************************************************************************
 * Menu
 *****************************************************************************/

void QLCWorkspace::slotRightMouseButtonClicked(const QPoint& pos)
{
	QPopupMenu* menu = NULL;
	QPopupMenu* themeMenu = NULL;
	QStyleFactory styleFactory;
	QStringList themeList = styleFactory.keys();
	QStringList::Iterator it;
	int i = 0;

	/* Master menu */
	menu = new QPopupMenu();
	menu->insertItem(QString("Workspace"), KMenuItemTitle);
	menu->insertSeparator();
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/image.png")),
			 "Background image", KMenuItemChangeBackground);

	/* Disable title */
	menu->setItemEnabled(KMenuItemTitle, false);

	/* Theme menu */
	themeMenu = new QPopupMenu();
	for (it = themeList.begin(); it != themeList.end(); ++it)
		themeMenu->insertItem(*it, KMenuItemChangeTheme + (i++));
	menu->insertItem("Visual Style", themeMenu);

	connect(menu, SIGNAL(activated(int)), 
		this, SLOT(slotMenuCallback(int)));
	connect(themeMenu, SIGNAL(activated(int)), 
		this, SLOT(slotMenuCallback(int)));

	menu->exec(pos);

	delete themeMenu;
	delete menu;
}

void QLCWorkspace::slotMenuCallback(int item)
{
	if (item == KMenuItemChangeBackground)
	{
		setBackgroundImage();
	}
	else if (item >= KMenuItemChangeTheme)
	{
		QStyleFactory styleFactory;
		QStringList themeList = styleFactory.keys();
		QString theme = themeList[item - KMenuItemChangeTheme];

		if (theme.isEmpty() == false)
			setTheme(theme);
	}
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void QLCWorkspace::mousePressEvent(QMouseEvent* event)
{
	if (event->button() & RightButton)
		emit rightMouseButtonClicked(mapToGlobal(event->pos()));
}
