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

#include "filehandler.h"
#include <qfile.h>
#include <qdom.h>

const QString KXMLCreatorNode    ( "Creator" );
const QString KXMLCreatorName    (    "Name" );
const QString KXMLCreatorVersion ( "Version" );
const QString KXMLCreatorAuthor  (  "Author" );

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
bool FileHandler::readXML(QString path, QDomDocument** document)
{
	QFile file(path);
	bool result = false;

	if (document != NULL && file.open(IO_ReadOnly) == true)
	{
		*document = new QDomDocument();
		result = (*document)->setContent(&file, false);
		file.close();
	}
	else
	{
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
	QDomElement root;
	QDomElement tag;
	QDomText text;

	if (doc == NULL || content == NULL)
		return false;

	*doc = new QDomDocument("QLCFile Content=\"" + content + "\"");

	/* Creator tag */
	root = (*doc)->createElement(KXMLCreatorNode);
	(*doc)->appendChild(root);

	/* Creator name */
	tag = (*doc)->createElement(KXMLCreatorName);
	root.appendChild(tag);
	text = (*doc)->createTextNode("Q Light Controller");
	tag.appendChild(text);

	/* Creator version */
	tag = (*doc)->createElement(KXMLCreatorVersion);
	root.appendChild(tag);
	text = (*doc)->createTextNode(QString(VERSION));
	tag.appendChild(text);

	/* Author */
	tag = (*doc)->createElement(KXMLCreatorAuthor);
	root.appendChild(tag);
	text = (*doc)->createTextNode(QString(getenv("USER")));
	tag.appendChild(text);
	
	return true;
}


