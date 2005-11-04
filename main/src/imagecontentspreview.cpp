/*
  Q Light Controller
  imagecontentspreview.cpp
  
  Copyright (C) Heikki Junnila
  
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

#include "imagecontentspreview.h"

#include <qfiledialog.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qstring.h>

ImageContentsPreview::ImageContentsPreview(QWidget* parent) 
  : QLabel(parent)
{
}
  
ImageContentsPreview::~ImageContentsPreview()
{
}

void ImageContentsPreview::previewUrl(const QUrl &u)
{
  QString path = u.path();
  QPixmap pix( path );
  if ( pix.isNull() )
    {
      setText("Not a valid image file");
    }
  else
    {
      setPixmap( pix );
    }
}
