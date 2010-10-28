/*
  Q Light Controller - Unit tests
  qlcfile_test.cpp

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,$
*/

#include <QtTest>
#include <QtXml>

#ifdef WIN32
#else
#   include <sys/types.h>
#   include <sys/stat.h>
#endif

#include "qlcfile_test.h"
#include "qlcfile.h"
#include "qlcconfig.h"

void QLCFile_Test::readXML()
{
    QDomDocument doc;

    doc = QLCFile::readXML(QString());
    QVERIFY(doc.isNull() == true);

    doc = QLCFile::readXML("foobar");
    QVERIFY(doc.isNull() == true);

    doc = QLCFile::readXML("broken.xml");
    QVERIFY(doc.isNull() == false);
    QCOMPARE(doc.firstChild().toElement().tagName(), QString("Workspace"));
    QCOMPARE(doc.firstChild().firstChild().toElement().tagName(), QString("Creator"));

	QString path("readonly.xml");
#ifndef WIN32
	QFile::Permissions perms = QFile::permissions(path);
	QFile::setPermissions(path, 0);
    doc = QLCFile::readXML(path);
    QVERIFY(doc.isNull() == true);
	QFile::setPermissions(path, perms);
#endif

    doc = QLCFile::readXML(path);
    QVERIFY(doc.isNull() == false);
    QCOMPARE(doc.firstChild().toElement().tagName(), QString("Workspace"));
    QCOMPARE(doc.firstChild().firstChild().toElement().tagName(), QString("Creator"));
}

void QLCFile_Test::getXMLHeader()
{
    bool insideCreatorTag = false;
    QDomDocument doc;

    doc = QLCFile::getXMLHeader(QString());
    QVERIFY(doc.isNull() == true);

    doc = QLCFile::getXMLHeader("Settings");
    QVERIFY(doc.isNull() == false);
    QCOMPARE(doc.doctype().name(), QString("Settings"));

    QDomNode node(doc.firstChild());
    QCOMPARE(node.toElement().tagName(), QString("Settings"));
    node = node.firstChild();
    QCOMPARE(node.toElement().tagName(), QString("Creator"));
    node = node.firstChild();
    while (node.isNull() == false)
    {
        // Verify that program enters this while loop
        insideCreatorTag = true;

        QDomElement tag(node.toElement());
        if (tag.tagName() == KXMLQLCCreatorAuthor)
            QCOMPARE(tag.text(), QString(getenv("USER")));
        else if (tag.tagName() == KXMLQLCCreatorName)
            QCOMPARE(tag.text(), QString(APPNAME));
        else if (tag.tagName() == KXMLQLCCreatorVersion)
            QCOMPARE(tag.text(), QString(APPVERSION));
        else
            QFAIL("Extra crap in XML header detected!");
        node = node.nextSibling();
    }

    QCOMPARE(insideCreatorTag, true);
}

void QLCFile_Test::errorString()
{
    QCOMPARE(QLCFile::errorString(QFile::NoError),
             tr("No error occurred."));
    QCOMPARE(QLCFile::errorString(QFile::ReadError),
             tr("An error occurred when reading from the file."));
    QCOMPARE(QLCFile::errorString(QFile::WriteError),
             tr("An error occurred when writing to the file."));
    QCOMPARE(QLCFile::errorString(QFile::FatalError),
             tr("A fatal error occurred."));
    QCOMPARE(QLCFile::errorString(QFile::ResourceError),
             tr("Resource error occurred."));
    QCOMPARE(QLCFile::errorString(QFile::OpenError),
             tr("The file could not be opened."));
    QCOMPARE(QLCFile::errorString(QFile::AbortError),
             tr("The operation was aborted."));
    QCOMPARE(QLCFile::errorString(QFile::TimeOutError),
             tr("A timeout occurred."));
    QCOMPARE(QLCFile::errorString(QFile::UnspecifiedError),
             tr("An unspecified error occurred."));
    QCOMPARE(QLCFile::errorString(QFile::RemoveError),
             tr("The file could not be removed."));
    QCOMPARE(QLCFile::errorString(QFile::RenameError),
             tr("The file could not be renamed."));
    QCOMPARE(QLCFile::errorString(QFile::PositionError),
             tr("The position in the file could not be changed."));
    QCOMPARE(QLCFile::errorString(QFile::ResizeError),
             tr("The file could not be resized."));
    QCOMPARE(QLCFile::errorString(QFile::PermissionsError),
             tr("The file could not be accessed."));
    QCOMPARE(QLCFile::errorString(QFile::CopyError),
             tr("The file could not be copied."));
    QCOMPARE(QLCFile::errorString(QFile::FileError(31337)),
             tr("An unknown error occurred."));
}
