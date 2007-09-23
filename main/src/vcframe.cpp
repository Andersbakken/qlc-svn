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

#include <qcursor.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <stdio.h>
#include <qbuttongroup.h>
#include <qobjectlist.h>

#include "vcframe.h"
#include "vcbutton.h"
#include "vclabel.h"
#include "vcxypad.h"
#include "vcdockslider.h"
#include "vcslider.h"

#include "app.h"
#include "doc.h"
#include "virtualconsole.h"
#include "vcframeproperties.h"
#include "common/filehandler.h"

extern App* _app;

VCFrame::VCFrame(QWidget* parent) : VCWidget(parent, "VCFrame")
{
	m_buttonBehaviour = Normal;
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
		resize(QPoint(120, 120));
	}
}

/*****************************************************************************
 * Bottom frame
 *****************************************************************************/

bool VCFrame::isBottomFrame()
{
	if (parentWidget() != NULL && parentWidget()->className() != "VCFrame")
		return true;
	else
		return false;
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCFrame::editProperties()
{
	VCFrameProperties* prop = new VCFrameProperties(this);
	prop->init();
	if (prop->exec() == QDialog::Accepted)
		_app->doc()->setModified();

	delete prop;
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
			QObjectList* l = _app->virtualConsole()
				->selectedWidget()->queryList("VCButton");
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
			QObjectList* l = _app->virtualConsole()
				->selectedWidget()->queryList("VCButton");
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
			{
				static_cast<VCFrame*> (child)->saveXML(doc, &root);
			}
			else if (child->className() == "VCButton")
			{
				static_cast<VCButton*> (child)->saveXML(doc, &root);
			}
			else if (child->className() == "VCDockSlider")
			{
				static_cast<VCDockSlider*> (child)->saveXML(doc, &root);
			}
			else if (child->className() == "VCLabel")
			{
				static_cast<VCLabel*> (child)->saveXML(doc, &root);
			}
			else if (child->className() == "VCXYPad")
			{
				static_cast<VCXYPad*> (child)->saveXML(doc, &root);
			}
			else
			{
				qWarning("Unknown VC widget class: %s",
					 child->className());
			}
			
			++it;
		}
	}

	return true;
}

/*****************************************************************************
 * Widget menu
 *****************************************************************************/

void VCFrame::invokeMenu(QPoint point)
{
	QPopupMenu* menu = VCWidget::createMenu();

	// Add menu
	QPopupMenu* addMenu = new QPopupMenu(menu);
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

	menu->insertSeparator();

	menu->insertItem("Add", addMenu);

	connect(addMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));

	menu->exec(point);

	delete menu;
}

void VCFrame::slotMenuCallback(int item)
{
	switch (item)
	{
	case KVCMenuAddButton:
		addButton();
		break;
	case KVCMenuAddSlider:
		addSlider();
		break;
	case KVCMenuAddFrame:
		addFrame();
		break;
	case KVCMenuAddXYPad:
		addXYPad();
		break;
	case KVCMenuAddLabel:
		addLabel();
		break;

	default:
		VCWidget::slotMenuCallback(item);
		break;
	}
}

/*****************************************************************************
 * Widget adding functions
 *****************************************************************************/

void VCFrame::addButton(QPoint at)
{
	VCButton* button = new VCButton(this);
	Q_ASSERT(button != NULL);
	button->init();
	button->show();

	if (this->buttonBehaviour() == VCFrame::Exclusive)
		button->setExclusive(true);
	else
		button->setExclusive(false);

	if (at.isNull() == false)
		button->move(at);
	else
		button->move(m_mousePressPoint);

	_app->doc()->setModified();
}

void VCFrame::addSlider(QPoint at)
{
/*
	VCDockSlider* slider = new VCDockSlider(this);
	Q_ASSERT(slider != NULL);
	slider->setBusID(KBusIDDefaultFade);
	slider->init();
	slider->resize(QPoint(55, 200));
	slider->show();
*/
	VCSlider* slider = new VCSlider(this);
	Q_ASSERT(slider != NULL);
	slider->init();
	slider->show();

	if (at.isNull() == false)
		slider->move(at);
	else
		slider->move(m_mousePressPoint);

	_app->doc()->setModified();
}

void VCFrame::addFrame(QPoint at)
{
	VCFrame* frame = new VCFrame(this);
	Q_ASSERT(frame != NULL);
	frame->init();
	frame->show();

	if (at.isNull() == false)
		frame->move(at);
	else
		frame->move(m_mousePressPoint);

	_app->doc()->setModified();
}

void VCFrame::addXYPad(QPoint at)
{
	VCXYPad* xypad = new VCXYPad(this);
	Q_ASSERT(xypad != NULL);
	xypad->init();
	xypad->show();

	if (at.isNull() == false)
		xypad->move(at);
	else
		xypad->move(m_mousePressPoint);

	_app->doc()->setModified();
}

void VCFrame::addLabel(QPoint at)
{
	VCLabel* label = new VCLabel(this);
	Q_ASSERT(label != NULL);
	//label->init();
	label->show();

	if (at.isNull() == false)
		label->move(at);
	else
		label->move(m_mousePressPoint);

	_app->doc()->setModified();
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

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
			m_mousePressPoint = QPoint(e->x(), e->y());
			invokeMenu(mapToGlobal(e->pos()));
		}
	}
	else
	{
		QFrame::mousePressEvent(e);
	}
}

void VCFrame::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
		slotMenuCallback(KVCMenuEditProperties);
}
