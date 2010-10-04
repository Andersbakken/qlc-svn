/*
  Q Light Controller - Unit test
  doc_test.h

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

#ifndef DOC_TEST_H
#define DOC_TEST_H

#include <QObject>
#include <QtXml>

#include "qlcfixturedefcache.h"

class Doc_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void defaults();

    void addFixture();
    void deleteFixture();
    void fixtureLimits();
    void fixture();
    void findAddress();
    void totalPowerConsumption();

    void addFunction();
    void deleteFunction();
    void function();
    void functionLimits();

    void load();
    void loadWrongRoot();
    void save();

private:
    QDomElement createFixtureNode(QDomDocument& doc, t_fixture_id id);
    QDomElement createCollectionNode(QDomDocument& doc, t_function_id id);
    QDomElement createBusNode(QDomDocument& doc, quint32 id, quint32 value);

private:
    QLCFixtureDefCache m_fixtureDefCache;
};

#endif
