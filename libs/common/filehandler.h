/*
  Q Light Controller
  filehandler.h
  
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

#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <qobject.h>
#include <qstring.h>
#include <qptrlist.h>

class QDomDocument;

// Generic tags common for all documents
#define KXMLQLCCreator "Creator"
#define KXMLQLCCreatorName "Name"
#define KXMLQLCCreatorVersion "Version"
#define KXMLQLCCreatorAuthor "Author"

class FileHandler : public QObject
{
	Q_OBJECT
		
public:
	/**
	 * Read an old QLC-style file to a list of key-value pairs
	 *
	 * @param fileName The name of the file to read
	 * @param list The resulting string list of key-value pairs
	 *
	 * @return true if succesful, otherwise false
	 */
	static bool readFileToList(QString &fileName, QPtrList <QString> &list);
	
	/**
	 * Read an XML file to a QDomDocument structure
	 *
	 * @param path Path to the file to read
	 * @param document QDomDocument* or NULL if an error has occurred
	 *
	 * @return true if succesful, otherwise false
	 */
	static bool readXML(QString path, QDomDocument** document);
	
	/**
	 * Get a common XML file header as a QDomDocument
	 *
	 * @param content The content type (Settings, Workspace)
	 * @param doc A newly-created QDomDocument containing the header
	 *
	 * @return true if succesful, otherwise false
	 */
	static bool getXMLHeader(QString content, QDomDocument** doc);
};
		
#endif
