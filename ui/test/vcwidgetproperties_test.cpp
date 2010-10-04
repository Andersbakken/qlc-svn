/*
  Q Light Controller - Unit test
  vcwidgetproperties_test.cpp

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

#include <QWidget>
#include <QtTest>

#include "vcwidgetproperties_test.h"
#include "vcwidgetproperties.h"

void VCWidgetProperties_Test::stateAndVisibility()
{
    VCWidgetProperties p;
    QVERIFY(p.state() == Qt::WindowNoState);
    QVERIFY(p.visible() == false);

    QWidget w(NULL);
    p.store(&w);
    QVERIFY(p.state() == Qt::WindowNoState);
    QVERIFY(p.visible() == false);

    w.showMinimized();
    p.store(&w);
    QVERIFY(p.state() & Qt::WindowMinimized);
    QVERIFY(p.visible() == true);

    w.showMaximized();
    p.store(&w);
    QVERIFY(p.state() & Qt::WindowMaximized);
    QVERIFY(p.visible() == true);

    w.showFullScreen();
    p.store(&w);
    QVERIFY(p.state() & Qt::WindowFullScreen);
    QVERIFY(p.visible() == true);

    w.hide();
    p.store(&w);
    QVERIFY(p.state() & Qt::WindowFullScreen);
    QVERIFY(p.visible() == false);
}

void VCWidgetProperties_Test::xy()
{
    VCWidgetProperties p;

    QWidget w(NULL);
    p.store(&w);

    QVERIFY(p.x() == 0);
    QVERIFY(p.y() == 0);

    w.move(50, 10);
    p.store(&w);

    QVERIFY(p.x() == 50);
    QVERIFY(p.y() == 10);
}

void VCWidgetProperties_Test::wh()
{
    VCWidgetProperties p;

    QWidget w(NULL);
    p.store(&w);

    QVERIFY(p.x() == 0);
    QVERIFY(p.y() == 0);

    w.resize(20, 30);
    p.store(&w);

    QVERIFY(p.width() == 20);
    QVERIFY(p.height() == 30);
}

void VCWidgetProperties_Test::load()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("WidgetProperties");
    doc.appendChild(root);

    QDomElement x = doc.createElement("X");
    QDomText xText = doc.createTextNode("50");
    x.appendChild(xText);
    root.appendChild(x);

    QDomElement y = doc.createElement("Y");
    QDomText yText = doc.createTextNode("70");
    y.appendChild(yText);
    root.appendChild(y);

    QDomElement w = doc.createElement("Width");
    QDomText wText = doc.createTextNode("40");
    w.appendChild(wText);
    root.appendChild(w);

    QDomElement h = doc.createElement("Height");
    QDomText hText = doc.createTextNode("60");
    h.appendChild(hText);
    root.appendChild(h);

    QDomElement v = doc.createElement("Visible");
    QDomText vText = doc.createTextNode("1");
    v.appendChild(vText);
    root.appendChild(v);

    QDomElement s = doc.createElement("State");
    QDomText sText = doc.createTextNode(QString("%1")
                                        .arg(Qt::WindowMinimized));
    s.appendChild(sText);
    root.appendChild(s);

    VCWidgetProperties p;
    p.loadXML(&root);
    QVERIFY(p.x() == 50);
    QVERIFY(p.y() == 70);
    QVERIFY(p.width() == 40);
    QVERIFY(p.height() == 60);
    QVERIFY(p.state() == Qt::WindowMinimized);
    QVERIFY(p.visible() == true);
}
