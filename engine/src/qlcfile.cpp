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

#include <QFile>
#include <QtXml>

#include "qlcconfig.h"
#include "qlctypes.h"
#include "qlcfile.h"

QFile::FileError QLCFile::readXML(const QString path, QDomDocument** doc)
{
	QFile::FileError error;
	QString msg;
	int line;
	int col;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(path != QString::null);

	QFile file(path);
	if (file.open(QIODevice::ReadOnly) == true)
	{
		*doc = new QDomDocument();
		if ((*doc)->setContent(&file, false, &msg, &line, &col) == true)
		{
			error = QFile::NoError;
		}
		else
		{
			qDebug() << path << ":" << msg << ", line:" << line
				 << ", col:" << col;
			error = QFile::ReadError;
		}
	}
	else
	{
		qDebug() << "Unable to open file:" << path;
		error = file.error();
	}

	file.close();

	return error;
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
	text = (*doc)->createTextNode(APPNAME);
	subtag.appendChild(text);

	/* Creator version */
	subtag = (*doc)->createElement(KXMLQLCCreatorVersion);
	tag.appendChild(subtag);
	text = (*doc)->createTextNode(QString(APPVERSION));
	subtag.appendChild(text);

	/* Author */
	subtag = (*doc)->createElement(KXMLQLCCreatorAuthor);
	tag.appendChild(subtag);
	text = (*doc)->createTextNode(QString(getenv("USER")));
	subtag.appendChild(text);

	return true;
}

QString QLCFile::errorString(QFile::FileError error)
{
	switch(error)
	{
	case QFile::NoError:
		return QObject::tr("No error occurred.");
	case QFile::ReadError:
		return QObject::tr("An error occurred when reading from the file.");
	case QFile::WriteError:
		return QObject::tr("An error occurred when writing to the file.");
	case QFile::FatalError:
		return QObject::tr("A fatal error occurred.");
	case QFile::ResourceError:
		return QObject::tr("Resource error occurred.");
	case QFile::OpenError:
		return QObject::tr("The file could not be opened.");
	case QFile::AbortError:
		return QObject::tr("The operation was aborted.");
	case QFile::TimeOutError:
		return QObject::tr("A timeout occurred.");
	case QFile::UnspecifiedError:
		return QObject::tr("An unspecified error occurred.");
	case QFile::RemoveError:
		return QObject::tr("The file could not be removed.");
	case QFile::RenameError:
		return QObject::tr("The file could not be renamed.");
	case QFile::PositionError:
		return QObject::tr("The position in the file could not be changed.");
	case QFile::ResizeError:
		return QObject::tr("The file could not be resized.");
	case QFile::PermissionsError:
		return QObject::tr("The file could not be accessed.");
	case QFile::CopyError:
		return QObject::tr("The file could not be copied.");
	default:
		return QObject::tr("An unknown error occurred.");
	}
}

