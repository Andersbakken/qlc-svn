#include <QWidget>
#include <QtTest>

#include "qlcwidgetproperties_test.h"
#include "../qlcwidgetproperties.h"

void QLCWidgetProperties_Test::stateAndVisibility()
{
	QLCWidgetProperties p;
	QVERIFY(p.state() == Qt::WindowNoState);
	QVERIFY(p.visible() == false);

	QWidget w(NULL);
	p.store(&w);
	QVERIFY(p.state() == Qt::WindowNoState);
	QVERIFY(p.visible() == false);

	w.showMinimized();
	p.store(&w);
	QVERIFY(p.state() == Qt::WindowMinimized);
	QVERIFY(p.visible() == true);

	w.showMaximized();
	p.store(&w);
	QVERIFY(p.state() == Qt::WindowMaximized);
	QVERIFY(p.visible() == true);

	w.showFullScreen();
	p.store(&w);
	QVERIFY(p.state() == Qt::WindowFullScreen);
	QVERIFY(p.visible() == true);

	w.hide();
	p.store(&w);
	QVERIFY(p.state() == Qt::WindowFullScreen);
	QVERIFY(p.visible() == false);
}

void QLCWidgetProperties_Test::xy()
{
	QLCWidgetProperties p;

	QWidget w(NULL);
	p.store(&w);

	QVERIFY(p.x() == 0);
	QVERIFY(p.y() == 0);

	w.move(50, 10);
	p.store(&w);

	QVERIFY(p.x() == 50);
	QVERIFY(p.y() == 10);
}

void QLCWidgetProperties_Test::wh()
{
	QLCWidgetProperties p;

	QWidget w(NULL);
	p.store(&w);

	QVERIFY(p.x() == 0);
	QVERIFY(p.y() == 0);

	w.resize(20, 30);
	p.store(&w);

	QVERIFY(p.width() == 20);
	QVERIFY(p.height() == 30);
}

void QLCWidgetProperties_Test::load()
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

	QLCWidgetProperties p;
	p.loadXML(&root);
	QVERIFY(p.x() == 50);
	QVERIFY(p.y() == 70);
	QVERIFY(p.width() == 40);
	QVERIFY(p.height() == 60);
	QVERIFY(p.state() == Qt::WindowMinimized);
	QVERIFY(p.visible() == true);
}
