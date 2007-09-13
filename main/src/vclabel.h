/*
  Q Light Controller
  vclabel.h

  Copyright (c) Heikki Junnila, Stefan Krumm
  
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

#ifndef VCLABEL_H
#define VCLABEL_H

#include "vcwidget.h"

class QDomDocument;
class QDomElement;

#define KXMLQLCVCLabel "Label"

class VCLabel : public VCWidget
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** Normal constructor */
	VCLabel(QWidget* parent);

	/** Destructor */
	~VCLabel();

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	static bool loader(QDomDocument* doc, QDomElement* root, QWidget* parent);
	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool saveXML(QDomDocument* doc, QDomElement* vc_root);

	/*********************************************************************
	 * Event handlers
	 *********************************************************************/
protected:
	void paintEvent(QPaintEvent* e);
};

#endif

