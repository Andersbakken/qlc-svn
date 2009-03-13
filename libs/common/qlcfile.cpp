/*
  Q Light Controller
  qlcfile.cpp

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

#include <iostream>
#include <QWidget>
#include <QFile>
#include <QtXml>

#include "qlctypes.h"
#include "qlcfile.h"

bool QLCFile::readXML(const QString path, QDomDocument** document)
{
	bool result = false;
	QString error;
	int line = 0;
	int col = 0;

	Q_ASSERT(document != NULL);
	Q_ASSERT(path != QString::null);

	QFile file(path);
	if (file.open(QIODevice::ReadOnly) == true)
	{
		*document = new QDomDocument();
		result = (*document)->setContent(&file, false,
						 &error, &line, &col);
		file.close();

		if (result == false)
		{
			qDebug() << path << ":" << error << ", line:" << line
				 << ", col:" << col;
		}
	}
	else
	{
		qDebug() << "Unable to open file:" << path;
		result = false;
	}

	return result;
}

bool QLCFile::getXMLHeader(QString content, QDomDocument** doc)
{
	QDomImplementation dom;
	QDomDocumentType doctype;
	QDomElement root;
	QDomElement tag;
	QDomElement subtag;
	QDomText text;

	if (doc == NULL || content == NULL)
		return false;

	doctype = dom.createDocumentType(content, QString::null, QString::null);
	*doc = new QDomDocument(doctype);

	root = (*doc)->createElement(content);
	(*doc)->appendChild(root);

	/* Creator tag */
	tag = (*doc)->createElement(KXMLQLCCreator);
	root.appendChild(tag);

	/* Creator name */
	subtag = (*doc)->createElement(KXMLQLCCreatorName);
	tag.appendChild(subtag);
	text = (*doc)->createTextNode("Q Light Controller");
	subtag.appendChild(text);

	/* Creator version */
	subtag = (*doc)->createElement(KXMLQLCCreatorVersion);
	tag.appendChild(subtag);
	text = (*doc)->createTextNode(QString(VERSION));
	subtag.appendChild(text);

	/* Author */
	subtag = (*doc)->createElement(KXMLQLCCreatorAuthor);
	tag.appendChild(subtag);
	text = (*doc)->createTextNode(QString(getenv("USER")));
	subtag.appendChild(text);

	return true;
}

bool QLCFile::saveXMLWindowState(QDomDocument* doc, QDomElement* root,
				 QWidget* window)
{
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	Q_ASSERT(window != NULL);

	if (doc == NULL || root == NULL || window == NULL)
		return false;

	/* Window state tag */
	tag = doc->createElement(KXMLQLCWindowState);
	root->appendChild(tag);

	/* Visible status */
	if (window->isVisible() == true)
		tag.setAttribute(KXMLQLCWindowStateVisible, KXMLQLCTrue);
	else
		tag.setAttribute(KXMLQLCWindowStateVisible, KXMLQLCFalse);

	/* X Coordinate */
	str.setNum(window->x());
	tag.setAttribute(KXMLQLCWindowStateX, str);

	/* Y Coordinate */
	str.setNum(window->y());
	tag.setAttribute(KXMLQLCWindowStateY, str);

	/* Width */
	str.setNum(window->width());
	tag.setAttribute(KXMLQLCWindowStateWidth, str);

	/* Height */
	str.setNum(window->height());
	tag.setAttribute(KXMLQLCWindowStateHeight, str);

	return true;
}

bool QLCFile::loadXMLWindowState(const QDomElement* tag,
				 int* x, int* y,
				 int* w, int* h,
				 bool* visible)
{
	if (tag == NULL || x == NULL || y == NULL || w == NULL || h == NULL ||
	    visible == NULL)
		return false;

	if (tag->tagName() == KXMLQLCWindowState)
	{
		*x = tag->attribute(KXMLQLCWindowStateX).toInt();
		*y = tag->attribute(KXMLQLCWindowStateY).toInt();
		*w = tag->attribute(KXMLQLCWindowStateWidth).toInt();
		*h = tag->attribute(KXMLQLCWindowStateHeight).toInt();

		if (tag->attribute(KXMLQLCWindowStateVisible) == KXMLQLCTrue)
			*visible = true;
		else
			*visible = false;

		return true;
	}
	else
	{
		qDebug() << "Window state not found!";
		return false;
	}
}
