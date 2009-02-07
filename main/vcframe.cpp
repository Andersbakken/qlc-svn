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

void VCFrame::slotDelete()
{
	/* Bottom frame cannot be destroyed */
	if (isBottomFrame() == true)
		return;

	if (QMessageBox::question(this, "Delete",
				  QString("Delete frame: %1?").arg(caption()),
				  QMessageBox::Yes,
				  QMessageBox::No) == QMessageBox::Yes)
	{
		_app->virtualConsole()->setSelectedWidget(NULL);
		_app->doc()->setModified();
		deleteLater();
	}
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCFrame::slotProperties()
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

	if (_app->virtualConsole()->selectedWidget() != NULL)
	{
		/* Find a list of child widgets, whose names match
		   the class name of VCButton (i.e. "VCButton") */
		QListIterator<VCButton*> it(
			_app->virtualConsole()->selectedWidget()
			->findChildren <VCButton*>(
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
 * Widget menu
 *****************************************************************************/

void VCFrame::invokeMenu(QPoint point)
{
	QMenu* addMenu;
	QMenu* menu;

	menu = VCWidget::createMenu();
	Q_ASSERT(menu != NULL);

	/* Create an "Add widget" menu */
	addMenu = new QMenu(menu);
	addMenu->setTitle("Add widget");
	addMenu->addAction(QIcon(":/button.png"), tr("Button"), 
			   this, SLOT(slotAddButton()));
	addMenu->addAction(QIcon(":/slider.png"), tr("Slider"),
			   this, SLOT(slotAddSlider()));
	addMenu->addAction(QIcon(":/xypad.png"), tr("XY pad"),
			   this, SLOT(slotAddXYPad()));
	addMenu->addSeparator();
	addMenu->addAction(QIcon(":/cuelist.png"), tr("Cue list"),
			   this, SLOT(slotAddCueList()));
	addMenu->addSeparator();
	addMenu->addAction(QIcon(":/frame.png"), tr("Frame"),
			   this, SLOT(slotAddFrame()));
	addMenu->addAction(QIcon(":/label.png"), tr("Label"),
			   this, SLOT(slotAddLabel()));

	/* Add sub menus to the master menu */
	menu->addSeparator();
	menu->addMenu(addMenu);

	/* Execute menu at the given point */
	menu->exec(point);

	/* Deletes also add menu and its actions */
	delete menu;
}

void VCFrame::slotAddButton()
{
	VCButton* button;
	QPoint at;

	button = new VCButton(this);
	Q_ASSERT(button != NULL);
	button->show();

	if (this->buttonBehaviour() == VCFrame::Exclusive)
		button->setExclusive(true);
	else
		button->setExclusive(false);

	if (at.isNull() == false)
		button->move(at);
	else
		button->move(m_mousePressPoint);

	_app->virtualConsole()->setSelectedWidget(button);

	_app->doc()->setModified();
}

void VCFrame::slotAddSlider()
{
	VCSlider* slider;
	QPoint at;

	slider = new VCSlider(this);
	Q_ASSERT(slider != NULL);
	slider->show();

	if (at.isNull() == false)
		slider->move(at);
	else
		slider->move(m_mousePressPoint);

	_app->virtualConsole()->setSelectedWidget(slider);

	_app->doc()->setModified();
}

void VCFrame::slotAddXYPad()
{
	VCXYPad* xypad;
	QPoint at;

	xypad = new VCXYPad(this);
	Q_ASSERT(xypad != NULL);
	xypad->show();

	if (at.isNull() == false)
		xypad->move(at);
	else
		xypad->move(m_mousePressPoint);

	_app->virtualConsole()->setSelectedWidget(xypad);

	_app->doc()->setModified();
}

void VCFrame::slotAddCueList()
{
	VCCueList* cuelist;
	QPoint at;

	cuelist = new VCCueList(this);
	Q_ASSERT(cuelist != NULL);
	cuelist->show();

	if (at.isNull() == false)
		cuelist->move(at);
	else
		cuelist->move(m_mousePressPoint);

	_app->virtualConsole()->setSelectedWidget(cuelist);

	_app->doc()->setModified();
}

void VCFrame::slotAddFrame()
{
	VCFrame* frame;
	QPoint at;

	frame = new VCFrame(this);
	Q_ASSERT(frame != NULL);
	frame->show();

	if (at.isNull() == false)
		frame->move(at);
	else
		frame->move(m_mousePressPoint);

	_app->virtualConsole()->setSelectedWidget(frame);

	_app->doc()->setModified();
}

void VCFrame::slotAddLabel()
{
	VCLabel* label;
	QPoint at;

	label = new VCLabel(this);
	Q_ASSERT(label != NULL);
	label->show();

	if (at.isNull() == false)
		label->move(at);
	else
		label->move(m_mousePressPoint);

	_app->virtualConsole()->setSelectedWidget(label);

	_app->doc()->setModified();
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCFrame::paintEvent(QPaintEvent* e)
{
	VCWidget::paintEvent(e);
}

void VCFrame::mouseMoveEvent(QMouseEvent* e)
{
	if (isBottomFrame() == false)
		VCWidget::mouseMoveEvent(e);
	else
		QWidget::mouseMoveEvent(e);
}
