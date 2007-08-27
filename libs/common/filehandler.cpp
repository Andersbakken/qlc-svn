/*
  Q Light Controller
  filehandler.cpp

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

#include <qfile.h>
#include <qdom.h>
#include <assert.h>
#include <qwidget.h>

#include "filehandler.h"
#include "settings.h"

/**
 * Read an old QLC-style file to a list of key-value pairs
 *
 * @param fileName The name of the file to read
 * @param list The resulting string list of key-value pairs
 *
 * @return true if succesful, otherwise false
 */
bool FileHandler::readFileToList(QString &fileName, QPtrList <QString> &list)
{
	QFile file(fileName);
	QString s = QString::null;
	QString t = QString::null;
	QString buf = QString::null;
	int i = 0;
	
	if (fileName == QString::null)
	{
		return false;
	}
	
	while (list.isEmpty() == false)
	{
		list.first();
		delete list.take();
	}
	
	if (file.open(IO_ReadOnly))
	{
		list.append(new QString("Entry"));
		list.append(new QString("Dummy"));
		
		// First read all entries to a string list
		while (file.atEnd() == false)
		{
			file.readLine(buf, 1024);
			
			// If there is no "equal" sign on this row or it begins
			// with a hash, ignore it
			i = buf.find(QString("="));
			if (i > -1 && buf.left(1) != QString("#"))
			{
				/* Get the string up to equal sign */
				s = buf.mid(0, i).stripWhiteSpace();
				list.append(new QString(s));
				
				/* Get the string after the equal sign */
				t = buf.mid(i + 1).stripWhiteSpace();
				list.append(new QString(t));
			}
		}
		
		file.close();
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Read an XML file to a QDomDocument structure
 *
 * @param path Path to the file to read
 * @param document QDomDocument* or NULL if an error has occurred
 *
 * @return true if succesful, otherwise false
 */
bool FileHandler::readXML(const QString path, QDomDocument** document)
{
	QFile file(path);
	bool result = false;
	QString error;
	int line = 0;
	int col = 0;

	assert(document != NULL);
	assert(path != QString::null);

	if (file.open(IO_ReadOnly) == true)
	{
		*document = new QDomDocument();
		result = (*document)->setContent(&file, false,
						 &error, &line, &col);
		file.close();

		if (result == false)
		{
			QString str;

			str.sprintf("%s: %s, line %d, col %d", path.ascii(),
				    error.ascii(), line, col);
			qDebug(str);
		}
	}
	else
	{
		qDebug(QString("Unable to open file: ") + path);
		result = false;
	}

	return result;
}


/**
 * Get a common XML file header as a QDomDocument
 *
 * @param content The content type (Settings, Workspace)
 * @param doc A newly-created QDomDocument containing the header
 *
 * @return true if succesful, otherwise false
 */
bool FileHandler::getXMLHeader(QString content, QDomDocument** doc)
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

bool FileHandler::saveXMLWindowState(QDomDocument* doc, QDomElement* root,
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
		tag.setAttribute(KXMLQLCWindowStateVisible,
				 Settings::trueValue());
	else
		tag.setAttribute(KXMLQLCWindowStateVisible,
				 Settings::falseValue());

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
