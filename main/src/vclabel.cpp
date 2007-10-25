/*
  Q Light Controller
  vclabel.cpp

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

#include <qstring.h>
#include <qpainter.h>

#include "vclabel.h"
#include "app.h"
#include "doc.h"
#include "virtualconsole.h"

#include "common/filehandler.h"

extern App* _app;

/**
 * TODO: VCLabel is pretty useless, because a VCFrame can do exactly
 * the same stuff that a label does. VCFrame could have a "label" property
 * which would make it impossible to add widgets to it...
 */
VCLabel::VCLabel(QWidget* parent) : VCWidget(parent, "Label")
{
	setCaption("Label");
	resize(QPoint(100, 30));
}

VCLabel::~VCLabel()
{
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCLabel::loader(QDomDocument* doc, QDomElement* root, QWidget* parent)
{
	VCLabel* label = NULL;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	Q_ASSERT(parent != NULL);

	if (root->tagName() != KXMLQLCVCLabel)
	{
		qWarning("Label node not found!");
		return false;
	}

	/* Create a new label into its parent */
	label = new VCLabel(parent);
	label->show();

	/* Continue loading */
	return label->loadXML(doc, root);
}

bool VCLabel::loadXML(QDomDocument* doc, QDomElement* root)
{
	bool visible = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	
	QDomNode node;
	QDomElement tag;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCLabel)
	{
		qWarning("Label node not found!");
		return false;
	}

	/* Caption */
	setCaption(root->attribute(KXMLQLCVCCaption));

	/* Children */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCWindowState)
		{
			FileHandler::loadXMLWindowState(&tag, &x, &y, &w, &h,
							&visible);
			setGeometry(x, y, w, h);
		}
		else if (tag.tagName() == KXMLQLCVCAppearance)
		{
			loadXMLAppearance(doc, &tag);
		}
		else
		{
			qWarning("Unknown label tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	return true;
}

bool VCLabel::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	/* VC Label entry */
	root = doc->createElement(KXMLQLCVCLabel);
	vc_root->appendChild(root);

	/* Caption */
	root.setAttribute(KXMLQLCVCCaption, caption());

	/* Window state */
	FileHandler::saveXMLWindowState(doc, &root, this);

	/* Appearance */
	saveXMLAppearance(doc, &root);

	return true;
}
