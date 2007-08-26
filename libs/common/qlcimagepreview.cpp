/*
  Q Light Controller
  qlcimagepreview.cpp
  
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

#include "qlcimagepreview.h"

#include <qfiledialog.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qimage.h>

QLCImagePreview::QLCImagePreview(QWidget* parent) 
	: QLabel(parent)
{
}

QLCImagePreview::~QLCImagePreview()
{
}

void QLCImagePreview::previewUrl(const QUrl &url)
{
	QPixmap pix(url.path());
	if (pix.isNull())
	{
		setText("Not a valid image file");
	}
	else
	{
		/* Resize all larger images to 200x200 */
		if (pix.width() > 200 || pix.height() > 200)
		{
			QImage image = pix.convertToImage();
			image = image.scale(200, 200, QImage::ScaleMin);
			pix.convertFromImage(image);
		}

		setPixmap(pix);
	}
}
