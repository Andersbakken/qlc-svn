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

#include <qpopupmenu.h>
#include <qpixmap.h>
#include <qfiledialog.h>

#include "common/settings.h"
#include "common/qlcworkspace.h"
#include "common/qlcimagepreview.h"

#define KMenuItemTitle            0
#define KMenuItemChangeBackground 1000

QLCWorkspace::QLCWorkspace(QWidget* parent)
	: QWorkspace(parent, "QLCWorkspace")
{
	connect(this, SIGNAL(rightMouseButtonClicked(const QPoint&)),
		this, SLOT(slotRightMouseButtonClicked(const QPoint&)));
}

QLCWorkspace::~QLCWorkspace()
{
}

void QLCWorkspace::setBackground(const QString& path)
{
	if (path != QString::null)
		setBackgroundPixmap(QPixmap(path));
	else
		setBackgroundPixmap(NULL);

	m_backgroundPath = path;
	
	emit backgroundChanged(path);
}

void QLCWorkspace::setBackground()
{
	QString path;

	QLCImagePreview* preview = new QLCImagePreview;
  
	QFileDialog* fd = new QFileDialog( this );
	fd->setCaption("Choose a background image");
	fd->setContentsPreviewEnabled(true);
	fd->setContentsPreview(preview, preview);
	fd->setPreviewMode(QFileDialog::Contents);
	fd->setFilter("Images (*.png *.xpm *.jpg *.gif)");
	fd->setSelection(m_backgroundPath);
  
	if (fd->exec() == QDialog::Accepted)
	{
		path = fd->selectedFile();
    
		if (path.isEmpty() == false)
		{
			setBackground(path);
		}
	}

	delete preview;
	delete fd;
}

void QLCWorkspace::slotRightMouseButtonClicked(const QPoint& pos)
{
	QPopupMenu* menu = NULL;

	menu = new QPopupMenu();
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/image.png")),
			 "Change background image", KMenuItemChangeBackground);

	menu->setItemEnabled(0, false);

	connect(menu, SIGNAL(activated(int)), 
		this, SLOT(slotMenuCallback(int)));

	menu->exec(pos);
	delete menu;
}

void QLCWorkspace::slotMenuCallback(int item)
{
	switch (item)
	{
	case KMenuItemChangeBackground:
		setBackground();
		break;
	default:
		break;
	}
}

void QLCWorkspace::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::RightButton)
		emit rightMouseButtonClicked(mapToGlobal(event->pos()));
}
