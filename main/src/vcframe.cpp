/*
  Q Light Controller
  vcframe.cpp

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

#include "vcframe.h"
#include "vcbutton.h"
#include "vclabel.h"
#include "vcxypad.h"
#include "vcdockslider.h"
#include "floatingedit.h"
#include "app.h"
#include "doc.h"
#include "virtualconsole.h"
#include "vcframeproperties.h"
#include "common/filehandler.h"

#include <qcursor.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qptrlist.h>
#include <stdio.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qbuttongroup.h>
#include <qobjectlist.h>
#include <assert.h>

extern App* _app;

VCFrame::VCFrame(QWidget* parent)
	: QFrame(parent, "VCFrame"),
	  m_backgroundImage  ( QString::null ),
	  m_hasCustomBackgroundColor ( false ),
	  m_hasCustomForegroundColor ( false ),
	  m_hasCustomFont ( false ),
	  m_xpos ( 0 ),
	  m_ypos ( 0 ),
	  m_resizeMode ( false ),
	  m_buttonBehaviour ( Normal )
{
}

VCFrame::~VCFrame()
{
}

void VCFrame::init(bool bottomFrame)
{
	if (bottomFrame == false)
	{
		setFrameStyle(KFrameStyleSunken);
		setMinimumSize(20, 20);
		resize(120, 120);
	}

	/* Listen to mode changes (operate/design) so menus can be en/disabled */
	disconnect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
	connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
}

bool VCFrame::isBottomFrame()
{
	if (parentWidget() != NULL && parentWidget()->className() != "VCFrame")
		return true;
	else
		return false;
}

/*********************************************************************
 * Background image
 *********************************************************************/
void VCFrame::setBackgroundImage(const QString& path)
{
	m_hasCustomBackgroundColor = false;
	m_backgroundImage = path;
	setPaletteBackgroundPixmap(QPixmap(path));
}

const QString& VCFrame::backgroundImage()
{
	return m_backgroundImage;
}

/*********************************************************************
 * Background color
 *********************************************************************/
void VCFrame::setBackgroundColor(const QColor& color)
{
	m_hasCustomBackgroundColor = true;
	m_backgroundImage = QString::null;
	setPaletteBackgroundColor(color);
}

void VCFrame::resetBackgroundColor()
{
	m_hasCustomBackgroundColor = false;
	/* TODO */
}

/*********************************************************************
 * Foreground color
 *********************************************************************/
void VCFrame::setForegroundColor(const QColor& color)
{
	m_hasCustomForegroundColor = true;
	setPaletteForegroundColor(color);
}

void VCFrame::resetForegroundColor()
{
	m_hasCustomForegroundColor = false;
	/* TODO */
}

/*********************************************************************
 * Font
 *********************************************************************/

void VCFrame::setFont(const QFont& font)
{
	m_hasCustomFont = true;
	QWidget::setFont(font);
}

void VCFrame::resetFont()
{
	m_hasCustomFont = false;
	/* TODO */
}

/*****************************************************************************
 * Button Behaviour
 *****************************************************************************/

void VCFrame::setButtonBehaviour(ButtonBehaviour b)
{
	m_buttonBehaviour = b;

	if (buttonBehaviour() == VCFrame::Exclusive)
	{
		if (_app->virtualConsole()->selectedWidget())
		{
			QObjectList* l = _app->virtualConsole()->selectedWidget()->queryList("VCButton");
			QObjectListIt it(*l);
			QObject *obj;
			while ((obj = it.current()) != 0)
			{
				++it;
				((VCButton*)obj)->setExclusive(true);
			}

			delete l;
			setFrameStyle(QFrame::GroupBoxPanel | QFrame::Sunken);
			setLineWidth(2);
		}
		else
		{
			setFrameStyle(QFrame::GroupBoxPanel | QFrame::Sunken);
			setLineWidth(2);
		}
	}
	else
	{
		if (_app->virtualConsole()->selectedWidget())
		{
			QObjectList* l = _app->virtualConsole()->selectedWidget()->queryList("VCButton");
			QObjectListIt it(*l);
			QObject *obj;
			while ((obj = it.current()) != 0)
			{
				++it;
				((VCButton*)obj)->setExclusive(false);
			}

			delete l;
			setFrameStyle(KFrameStyleSunken);
			setLineWidth(1);
		}
		else
		{
			setFrameStyle(KFrameStyleSunken);
			setLineWidth(1);
		}
	}
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCFrame::loader(QDomDocument* doc, QDomElement* root, QWidget* parent)
{
	VCFrame* frame = NULL;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	Q_ASSERT(parent != NULL);

	if (root->tagName() != KXMLQLCVCFrame)
	{
		qWarning("Frame node not found!");
		return false;
	}

	/* Create a new frame into its parent */
	frame = new VCFrame(parent);
	frame->show();

	/* If the current parent widget is anything else than VCFrame,
	   the currently loaded VCFrame is the parent of all VC widgets */
	if (parent->className() != "VCFrame")
		_app->virtualConsole()->setDrawArea(frame);
	else
		frame->init();

	/* Continue loading */
	return frame->loadXML(doc, root);
}

bool VCFrame::loadXML(QDomDocument* doc, QDomElement* root)
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

	if (root->tagName() != KXMLQLCVCFrame)
	{
		qWarning("Frame node not found!");
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
		else if (tag.tagName() == KXMLQLCVCFrame)
		{
			VCFrame::loader(doc, &tag, this);
		}
		else if (tag.tagName() == KXMLQLCVCLabel)
		{
			VCLabel::loader(doc, &tag, this);
		}
		else if (tag.tagName() == KXMLQLCVCButton)
		{
			VCButton::loader(doc, &tag, this);
		}
		else if (tag.tagName() == KXMLQLCVCXYPad)
		{
			VCXYPad::loader(doc, &tag, this);
		}
		else if (tag.tagName() == KXMLQLCVCDockSlider)
		{
			VCDockSlider::loader(doc, &tag, this);
		}
		else
		{
			qWarning("Unknown frame tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	return true;
}

bool VCFrame::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
	const QObjectList* objectList = NULL;
	QObject* child = NULL;
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	/* VC Frame entry */
	root = doc->createElement(KXMLQLCVCFrame);
	vc_root->appendChild(root);

	/* Caption */
	root.setAttribute(KXMLQLCVCCaption, caption());

	/* Button Behaviour */
	str.setNum(buttonBehaviour());
	root.setAttribute(KXMLQLCVCFrameButtonBehaviour, str);

	/* Save appearance */
	saveXMLAppearance(doc, &root);

	/* Save widget proportions only for child frames */
	if (isBottomFrame() == false)
		FileHandler::saveXMLWindowState(doc, &root, this);

	/* Save children */
	objectList = children();
	if (objectList != NULL)
	{
		QObjectListIterator it(*objectList);
		while ( (child = it.current()) != NULL )
		{
			if (child->className() == "VCFrame")
				static_cast<VCFrame*> (child)->saveXML(doc, &root);
			else if (child->className() == "VCButton")
				static_cast<VCButton*> (child)->saveXML(doc, &root);
			else if (child->className() == "VCDockSlider")
				static_cast<VCDockSlider*> (child)->saveXML(doc, &root);
			else if (child->className() == "VCLabel")
				static_cast<VCLabel*> (child)->saveXML(doc, &root);
			else if (child->className() == "VCXYPad")
				static_cast<VCXYPad*> (child)->saveXML(doc, &root);
			else
				qWarning("Unknown widget class: %s", child->className());
			
			++it;
		}
	}

	return true;
}

bool VCFrame::loadXMLAppearance(QDomDocument* doc, QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCAppearance)
	{
		qWarning("Appearance node not found!");
		return false;
	}

	/* Children */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCVCFrameStyle)
		{
			int style = 0;
			style = VirtualConsole::stringToFrameStyle(tag.text());
			setFrameStyle(style);
		}
		else if (tag.tagName() == KXMLQLCVCForegroundColor)
		{
			if (tag.text() != KXMLQLCVCColorDefault)
			{
				QColor color(tag.text().toInt());
				setForegroundColor(color);
			}
		}
		else if (tag.tagName() == KXMLQLCVCBackgroundColor)
		{
			if (tag.text() != KXMLQLCVCColorDefault)
				setBackgroundColor(QColor(tag.text().toInt()));
		}
		else if (tag.tagName() == KXMLQLCVCBackgroundImage)
		{
			if (tag.text() != KXMLQLCVCBackgroundImageNone)
				setBackgroundImage(tag.text());
		}
		else if (tag.tagName() == KXMLQLCVCFont)
		{
			if (tag.text() != KXMLQLCVCFontDefault)
			{
				QFont font;
				font.fromString(tag.text());
				setFont(font);
			}
		}
		else
		{
			qWarning("Unknown Appearance tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}
}

bool VCFrame::saveXMLAppearance(QDomDocument* doc, QDomElement* frame_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(frame_root != NULL);

	/* VC Label entry */
	root = doc->createElement(KXMLQLCVCAppearance);
	frame_root->appendChild(root);

	/* Frame style */
	tag = doc->createElement(KXMLQLCVCFrameStyle);
	root.appendChild(tag);
	text = doc->createTextNode(VirtualConsole::frameStyleToString(frameStyle()));
	tag.appendChild(text);

	/* Foreground color */
	tag = doc->createElement(KXMLQLCVCForegroundColor);
	root.appendChild(tag);
	if (hasCustomForegroundColor() == true)
		str.setNum(paletteForegroundColor().rgb());
	else
		str = KXMLQLCVCColorDefault;
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Background color */
	tag = doc->createElement(KXMLQLCVCBackgroundColor);
	root.appendChild(tag);
	if (hasCustomBackgroundColor() == true)
		str.setNum(paletteBackgroundColor().rgb());
	else
		str = KXMLQLCVCColorDefault;
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Background image */
	tag = doc->createElement(KXMLQLCVCBackgroundImage);
	root.appendChild(tag);
	if (backgroundImage() != QString::null)
		str = m_backgroundImage;
	else
		str = KXMLQLCVCBackgroundImageNone;
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Font */
	tag = doc->createElement(KXMLQLCVCFont);
	root.appendChild(tag);
	if (hasCustomFont() == true)
		str = font().toString();
	else
		str = KXMLQLCVCFontDefault;
	text = doc->createTextNode(str);
	tag.appendChild(text);	

	return true;
}

/*********************************************************************
 *
 *********************************************************************/

void VCFrame::paintEvent(QPaintEvent* e)
{
	QFrame::paintEvent(e);

	QPainter p(this);

	if (_app->mode() == App::Design &&
	    _app->virtualConsole()->selectedWidget() == this)
	{
		// Draw a dotted line around the widget
		QPen pen(DotLine);
		pen.setWidth(2);
		p.setPen(pen);
		p.drawRect(1, 1, rect().width() - 1, rect().height() - 1);

		// Draw a resize handle
		QBrush b(SolidPattern);
		p.fillRect(rect().width() - 10, rect().height() - 10, 10, 10, b);
	}
}

void VCFrame::slotModeChanged()
{
	repaint();
}

void VCFrame::mousePressEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		_app->virtualConsole()->setSelectedWidget(this);

		if (m_resizeMode == true && isBottomFrame() == false)
		{
			setMouseTracking(false);
			m_resizeMode = false;
		}

		if ((e->button() & LeftButton || e->button() & MidButton)
		    && isBottomFrame() == false)
		{
			if (e->x() > rect().width() - 10 &&
			    e->y() > rect().height() - 10)
			{
				m_resizeMode = true;
				setMouseTracking(true);
				setCursor(QCursor(SizeFDiagCursor));
			}
			else
			{
				m_mousePressPoint = QPoint(e->x(), e->y());
				setCursor(QCursor(SizeAllCursor));
			}
		}
		else if (e->button() & RightButton)
		{
			invokeMenu(mapToGlobal(e->pos()));
		}
	}
	else
	{
		QFrame::mousePressEvent(e);
	}
}

void VCFrame::invokeMenu(QPoint point)
{
	//
	// Add menu
	//
	QPopupMenu* addMenu = new QPopupMenu();
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/button.png")),
			    "&Button", KVCMenuAddButton);
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/slider.png")),
			    "&Slider", KVCMenuAddSlider);
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/frame.png")),
			    "&Frame", KVCMenuAddFrame);
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/xypad.png")),
			    "&XY-Pad", KVCMenuAddXYPad);
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/label.png")),
			    "L&abel", KVCMenuAddLabel);

	QPopupMenu* menu = new QPopupMenu();
	// Insert common stuff from virtual console (100% dirty hack)
	menu->insertItem("Edit", _app->virtualConsole()->editMenu());

	// Insert the add menu
	menu->insertItem("Add", addMenu);

	switch(menu->exec(point))
	{
	case KVCMenuAddButton:
		slotAddButton(mapFromGlobal(point));
		break;

	case KVCMenuAddSlider:
		slotAddSlider(mapFromGlobal(point));
		break;

	case KVCMenuAddFrame:
		slotAddFrame(mapFromGlobal(point));
		break;

	case KVCMenuAddXYPad:
		slotAddXYPad(mapFromGlobal(point));
		break;

	case KVCMenuAddLabel:
		slotAddLabel(mapFromGlobal(point));
		break;

	default:
		break;
	}

	delete addMenu;
	delete menu;
}

void VCFrame::slotAddButton(QPoint p)
{
	VCButton* b = new VCButton(this);
	assert(b);
	b->init();
	b->show();

	if (buttonBehaviour() == VCFrame::Exclusive)
	{
		b->setExclusive(true);
	}
	else
	{
		b->setExclusive(false);
	}

	b->move(p);

	_app->doc()->setModified();
}

void VCFrame::slotAddSlider(QPoint p)
{
	VCDockSlider* s = new VCDockSlider(this);
	assert(s);
	s->setBusID(KBusIDDefaultFade);
	s->init();
	s->resize(55, 200);
	s->show();

	s->move(p);

	_app->doc()->setModified();
}

void VCFrame::slotAddFrame(QPoint p)
{
	VCFrame* f = new VCFrame(this);
	assert(f);
	f->init();
	f->show();

	f->move(p);

	_app->doc()->setModified();
}

void VCFrame::slotAddXYPad(QPoint p)
{
	VCXYPad* x = new VCXYPad(this);
	assert(x);
	x->init();
	x->show();

	x->move(p);

	_app->doc()->setModified();
}

void VCFrame::slotAddLabel(QPoint p)
{
	VCLabel* l = new VCLabel(this);
	assert(l);
	l->init();
	l->show();

	l->move(p);

	_app->doc()->setModified();
}




void VCFrame::parseWidgetMenu(int item)
{
	switch (item)
	{
	case KVCMenuEditProperties:
	{
		VCFrameProperties* vcfp = new VCFrameProperties(this);
		vcfp->init();
		if (vcfp->exec() == QDialog::Accepted)
		{
			_app->doc()->setModified();
		}

		delete vcfp;
	}
	break;

	case KVCMenuBackgroundFrame:
	{
		if (frameStyle() & KFrameStyleSunken)
		{
			setFrameStyle(NoFrame);
		}
		else
		{
			setFrameStyle(KFrameStyleSunken);
		}
		_app->doc()->setModified();
	}
	break;

	default:
		break;
	}
}

void VCFrame::mouseReleaseEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		unsetCursor();
		m_resizeMode = false;
		setMouseTracking(false);
	}
	else
	{
		QFrame::mouseReleaseEvent(e);
	}
}

void VCFrame::mouseMoveEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		if (m_resizeMode == true)
		{
			QPoint p(QCursor::pos());
			resizeTo(mapFromGlobal(p));
			_app->doc()->setModified();
		}
		else if (e->state() & LeftButton || e->state() & MidButton)
		{
			QPoint p(parentWidget()->mapFromGlobal(QCursor::pos()));
			p.setX(p.x() - m_mousePressPoint.x());
			p.setY(p.y() - m_mousePressPoint.y());

			moveTo(p);
			_app->doc()->setModified();
		}
	}
	else
	{
		QFrame::mouseMoveEvent(e);
	}
}

void VCFrame::customEvent(QCustomEvent* e)
{
	if (e->type() == KVCMenuEvent)
	{
		parseWidgetMenu(((VCMenuEvent*) e)->menuItem());
	}
}


void VCFrame::resizeTo(QPoint p)
{
	// Grid settings
	if (_app->virtualConsole()->isGridEnabled())
	{
		p.setX(p.x() - (p.x() % _app->virtualConsole()->gridX()));
		p.setY(p.y() - (p.y() % _app->virtualConsole()->gridY()));
	}

	// Map to parent coordinates so that they can be compared
	p = mapToParent(p);

	// Don't move beyond left or right
	if (p.x() < 0)
	{
		p.setX(0);
	}
	else if (p.x() > parentWidget()->width())
	{
		p.setX(parentWidget()->width());
	}

	// Don't move beyond top or bottom
	if (p.y() < 0)
	{
		p.setY(0);
	}
	else if (p.y() > parentWidget()->height())
	{
		p.setY(parentWidget()->height());
	}

	// Map back so that this can be resized
	p = mapFromParent(p);

	// Do the resize
	resize(p.x(), p.y());
}


void VCFrame::moveTo(QPoint p)
{
	// Grid settings
	if (_app->virtualConsole()->isGridEnabled())
	{
		p.setX(p.x() - (p.x() % _app->virtualConsole()->gridX()));
		p.setY(p.y() - (p.y() % _app->virtualConsole()->gridY()));
	}

	// Don't move beyond left or right
	if (p.x() < 0)
	{
		p.setX(0);
	}
	else if (p.x() + rect().width() > parentWidget()->width())
	{
		p.setX(parentWidget()->width() - rect().width());
	}

	// Don't move beyond top or bottom
	if (p.y() < 0)
	{
		p.setY(0);
	}
	else if (p.y() + rect().height() > parentWidget()->height())
	{
		p.setY(parentWidget()->height() - rect().height());
	}

	// Do the move
	move(p);
}

void VCFrame::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		invokeMenu(mapToGlobal(e->pos()));
	}
	else
	{
		mousePressEvent(e);
	}
}
