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

#include <QStyleOptionFrameV2>
#include <QMetaObject>
#include <QMessageBox>
#include <QPainter>
#include <QAction>
#include <QStyle>
#include <QDebug>
#include <QPoint>
#include <QSize>
#include <QMenu>
#include <QList>
#include <QtXml>

#include "common/qlcfile.h"

#include "vcframeproperties.h"
#include "virtualconsole.h"
#include "vccuelist.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
#include "vclabel.h"
#include "vcxypad.h"
#include "app.h"
#include "doc.h"

extern App* _app;

VCFrame::VCFrame(QWidget* parent) : VCWidget(parent)
{
	/* Set the class name "VCFrame" as the object name as well */
	setObjectName(VCFrame::staticMetaObject.className());

	m_buttonBehaviour = Normal;
	m_frameStyle = KVCFrameStyleSunken;

	setMinimumSize(20, 20);
	QWidget::resize(QSize(120, 120));
}

VCFrame::~VCFrame()
{
}

bool VCFrame::isBottomFrame()
{
	/* If this widget has a parent that is NOT a VCFrame, this widget
	   is the bottom frame. */
	if (parentWidget() != NULL &&
	    QString(parentWidget()->metaObject()->className()) !=
	    QString(VCFrame::staticMetaObject.className()))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCFrame::createCopy(VCWidget* parent)
{
	Q_ASSERT(parent != NULL);

	VCFrame* frame = new VCFrame(parent);
	if (frame->copyFrom(this) == false)
	{
		delete frame;
		frame = NULL;
	}

	return frame;
}

bool VCFrame::copyFrom(VCWidget* widget)
{
	VCFrame* frame = qobject_cast<VCFrame*> (widget);
	if (frame == NULL)
		return false;

	/* Copy button behaviour */
	setButtonBehaviour(frame->buttonBehaviour());

	/* TODO: Copy children? */

	/* Copy common stuff */
	return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCFrame::editProperties()
{
	VCFrameProperties prop(_app, this);
	if (prop.exec() == QDialog::Accepted)
		_app->doc()->setModified();
}

/*****************************************************************************
 * Button Behaviour
 *****************************************************************************/

void VCFrame::setButtonBehaviour(ButtonBehaviour b)
{
	m_buttonBehaviour = b;

	/* Find a list of child widgets, whose names match
	   the class name of VCButton (i.e. "VCButton") */
	QListIterator<VCButton*> it(findChildren <VCButton*>(
				      VCButton::staticMetaObject.className()));

	if (b == VCFrame::Exclusive)
	{
		while (it.hasNext() == true)
			it.next()->setExclusive(true);
	}
	else
	{
		while (it.hasNext() == true)
			it.next()->setExclusive(false);
	}
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCFrame::loader(QDomDocument* doc, QDomElement* root, QWidget* parent)
{
	VCFrame* frame = NULL;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	Q_ASSERT(parent != NULL);

	if (root->tagName() != KXMLQLCVCFrame)
	{
		qDebug() << "Frame node not found!";
		return false;
	}

	/* Create a new frame into its parent */
	frame = new VCFrame(parent);
	frame->show();

	/* If the current parent widget is anything else than a VCFrame,
	   the currently loaded VCFrame becomes the parent of all VC widgets */
	if (parent->objectName() != VCFrame::staticMetaObject.className())
		_app->virtualConsole()->setDrawArea(frame);

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
		qDebug() << "Frame node not found!";
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
			QLCFile::loadXMLWindowState(&tag, &x, &y, &w, &h,
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
		else if (tag.tagName() == KXMLQLCVCSlider)
		{
			VCSlider::loader(doc, &tag, this);
		}
		else if (tag.tagName() == KXMLQLCVCCueList)
		{
			VCCueList::loader(doc, &tag, this);
		}
		else
		{
			qDebug() << "Unknown frame tag:" << tag.tagName();
		}
		
		node = node.nextSibling();
	}

	return true;
}

bool VCFrame::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
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
		QLCFile::saveXMLWindowState(doc, &root, this);

	/* Save children */
	QListIterator <VCWidget*> it(findChildren<VCWidget*>());
	while (it.hasNext() == true)
	{
		VCWidget* widget = it.next();

		/* findChildren() is recursive, so the list contains all
		   possible child widgets below this frame. Each frame must
		   save only its direct children to preserve hierarchy, so
		   save only such widgets that have this widget as their
		   direct parent. */
		if (widget->parentWidget() == this)
			widget->saveXML(doc, &root);
	}

	return true;
}

/*****************************************************************************
 * Custom menu
 *****************************************************************************/

QMenu* VCFrame::customMenu(QMenu* parentMenu)
{
	/* Basically, just returning VC::addMenu() would suffice here, but
	   since the returned menu will be deleted when the current widget
	   changes, we have to copy the menu's contents into a new menu. */
	QMenu* menu = new QMenu(parentMenu);
	menu->setTitle(tr("Add"));
	QListIterator <QAction*> it(
				_app->virtualConsole()->addMenu()->actions());
	while (it.hasNext() == true)
		menu->addAction(it.next());
	return menu;
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCFrame::handleWidgetSelection(QMouseEvent* e)
{
	/* Don't allow selection of the bottom frame. Selecting it will always
	   actually clear the current selection. */
	if (isBottomFrame() == false)
		VCWidget::handleWidgetSelection(e);
	else
		_app->virtualConsole()->clearWidgetSelection(); 
}

void VCFrame::mouseMoveEvent(QMouseEvent* e)
{
	if (isBottomFrame() == false)
		VCWidget::mouseMoveEvent(e);
	else
		QWidget::mouseMoveEvent(e);
}
