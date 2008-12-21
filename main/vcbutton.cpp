/*
  Q Light Controller
  vcbutton.cpp

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

#include <QPaintEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>
#include <QString>
#include <QDebug>
#include <QEvent>
#include <QTimer>
#include <QBrush>
#include <QtXml>
#include <QMenu>
#include <QSize>
#include <QPen>

#include "common/qlcfile.h"

#include "vcbuttonproperties.h"
#include "functionselection.h"
#include "virtualconsole.h"
#include "inputmap.h"
#include "vcbutton.h"
#include "function.h"
#include "vcframe.h"
#include "fixture.h"
#include "keybind.h"
#include "app.h"
#include "doc.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCButton::VCButton(QWidget* parent) : VCWidget(parent)
{
	/* Set the class name "VCButton" as the object name as well */
	setObjectName(VCButton::staticMetaObject.className());

	/* No function is initially attached to the button */
	m_function = KNoID;

	/* Set the frame line width a bit thicker to make this look more
	   like a button than a frame (which it actually is) */
	setLineWidth(2);

	setCaption(QString::null);
	setOn(false);
	setExclusive(false);
	setStopFunctions(false);

	/* Initial size */
	setMinimumSize(20, 20);
	resize(QPoint(30, 30));

	/* Keybinding */
	m_keyBind = new KeyBind();
	connect(m_keyBind, SIGNAL(pressed()), this, SLOT(pressFunction()));
	connect(m_keyBind, SIGNAL(released()), this, SLOT(releaseFunction()));
}

VCButton::~VCButton()
{
	if (m_keyBind != NULL)
		delete m_keyBind;
	m_keyBind = NULL;
}

void VCButton::slotDelete()
{
	if (QMessageBox::question(this, "Delete",
				  QString("Delete button: %1?")
				  .arg(caption()),
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

void VCButton::slotProperties()
{
	VCButtonProperties prop(this, _app);
	prop.exec();
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCButton::loader(QDomDocument* doc, QDomElement* root, QWidget* parent)
{
	VCButton* button = NULL;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	Q_ASSERT(parent != NULL);

	if (root->tagName() != KXMLQLCVCButton)
	{
		qDebug() << "Button node not found!";
		return false;
	}

	/* Create a new button into its parent */
	button = new VCButton(parent);
	button->show();

	/* Continue loading */
	return button->loadXML(doc, root);
}

bool VCButton::loadXML(QDomDocument* doc, QDomElement* root)
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

	if (root->tagName() != KXMLQLCVCButton)
	{
		qDebug() << "Button node not found!";
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
		else if (tag.tagName() == KXMLQLCVCButtonFunction)
		{
			str = tag.attribute(KXMLQLCVCButtonFunctionID);
			setFunction(str.toInt());
		}
		else if (tag.tagName() == KXMLQLCVCWidgetInput)
		{
			loadXMLInput(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCKeyBind)
		{
			KeyBind* kb = new KeyBind();
			kb->loadXML(doc, &tag);
			setKeyBind(kb);
			delete kb;
		}
		else
		{
			qDebug() << "Unknown button tag:" << tag.tagName();
		}

		node = node.nextSibling();
	}

	/* All buttons start raised... */
	setOn(false);

	return true;
}

bool VCButton::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	/* VC Label entry */
	root = doc->createElement(KXMLQLCVCButton);
	vc_root->appendChild(root);

	/* Caption */
	root.setAttribute(KXMLQLCVCCaption, caption());

	/* Function */
	tag = doc->createElement(KXMLQLCVCButtonFunction);
	root.appendChild(tag);
	str.setNum(function());
	tag.setAttribute(KXMLQLCVCButtonFunctionID, str);

	/* External input */
	saveXMLInput(doc, &root);

	/* Window state */
	QLCFile::saveXMLWindowState(doc, &root, this);

	/* Appearance */
	saveXMLAppearance(doc, &root);

	/* Key binding */
	m_keyBind->saveXML(doc, &root);

	return true;
}

/*****************************************************************************
 * Button state
 *****************************************************************************/

void VCButton::setOn(bool on)
{
	m_on = on;

	/* Simulate button-look with a frame (line width has been set a bit
	   thicker in constructor to emphasize this) */
	if (on == true)
		VCWidget::setFrameStyle(QFrame::Panel | QFrame::Sunken);
	else
		VCWidget::setFrameStyle(QFrame::Panel | QFrame::Raised);

	/* Send input feedback */
	if (m_inputUniverse != KInputUniverseInvalid &&
	    m_inputChannel != KInputChannelInvalid)
	{
		if (on == true)
		{
			_app->inputMap()->feedBack(m_inputUniverse,
						   m_inputChannel,
						   KInputValueMax);
		}
		else
		{
			_app->inputMap()->feedBack(m_inputUniverse,
						   m_inputChannel,
						   0);
		}
	}
}

void VCButton::setFrameStyle(const int)
{
	/* Don't allow real frame style setting because we need to simulate
	   button-look with a custom frame style */
}

/*****************************************************************************
 * KeyBind
 *****************************************************************************/

void VCButton::setKeyBind(const KeyBind* kb)
{
	Q_ASSERT(kb != NULL);

	if (m_keyBind != NULL)
		delete m_keyBind;
	m_keyBind = new KeyBind(kb);

	connect(m_keyBind, SIGNAL(pressed()), this, SLOT(pressFunction()));
	connect(m_keyBind, SIGNAL(released()), this, SLOT(releaseFunction()));
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VCButton::slotInputValueChanged(t_input_universe universe,
				     t_input_channel channel,
				     t_input_value value)
{
	/* Don't allow operation during design mode */
	if (_app->mode() == App::Design)
		return;

	if (universe == m_inputUniverse && channel == m_inputChannel)
		pressFunction();
}

/*****************************************************************************
 * Function attachment
 *****************************************************************************/

void VCButton::setFunction(t_function_id fid)
{
	Function* function = _app->doc()->function(fid);

	if (function != NULL)
	{
		Function* old = _app->doc()->function(m_function);
		if (old != NULL)
		{
			disconnect(old, SIGNAL(running(t_function_id)),
				   this, SLOT(slotFunctionRunning(t_function_id)));
			disconnect(old, SIGNAL(stopped(t_function_id)),
				   this, SLOT(slotFunctionStopped(t_function_id)));
		}

		connect(function, SIGNAL(running(t_function_id)),
			this, SLOT(slotFunctionRunning(t_function_id)));
		connect(function, SIGNAL(stopped(t_function_id)),
			this, SLOT(slotFunctionStopped(t_function_id)));

		m_function = fid;

		setToolTip(function->name());
	}
	else
	{
		m_function = KNoID;
		setToolTip(QString::null);
	}

	_app->doc()->setModified();
}

void VCButton::setExclusive(bool exclusive)
{
	// sure, we have made this over the frame stuff
	// but it works for the moment and can change sometime
	m_isExclusive = exclusive;
}

/*****************************************************************************
 * Button press / release handlers
 *****************************************************************************/

void VCButton::slotFunctionRunning(t_function_id fid)
{
	if (fid == m_function)
		setOn(true);
}

void VCButton::slotFunctionStopped(t_function_id fid)
{
	if (fid == m_function)
	{
		setOn(false);
		slotFlashReady();
		QTimer::singleShot(200, this, SLOT(slotFlashReady()));
	}
}

void VCButton::pressFunction()
{
	Function* f = NULL;

	/* TODO: Should this return immediately? */
	if (m_stopFunctions == true)
		_app->slotControlPanic();

	if (m_function == KNoID)
	{
		return;
	}
	else if (m_keyBind->action() == KeyBind::Toggle &&
		 m_isExclusive == false)
	{
		f = _app->doc()->function(m_function);
		if (f != NULL)
		{
			if (isOn() == true)
				f->stop();
			else
				f->start();
		}
		else
		{
			qDebug() << "Function has been deleted!";
			setFunction(KNoID);
		}
	}
	else if (m_keyBind->action() == KeyBind::Toggle &&
		 m_isExclusive == true)
	{
		/* Get a list of this button's siblings from this' parent */
		QListIterator <VCButton*> it(
			parentWidget()->findChildren<VCButton*>("VCButton"));

		/* Stop all sibling buttons' functions */
		while (it.hasNext() == true)
		{
			VCButton* sibling = it.next();
			if (sibling != this &&
			    sibling->parentWidget() == parentWidget())
			{
				sibling->setOn(false);
			}
		}

		/* Start this button's function */
		f = _app->doc()->function(m_function);
		if (f != NULL)
		{
			f->start();
		}
		else
		{
			qDebug() << "Function has been deleted!";
			setFunction(KNoID);
		}
	}
	else if (m_keyBind->action() == KeyBind::Flash)
	{
		f = _app->doc()->function(m_function);
		if (f != NULL)
		{
			f->start();
		}
		else
		{
			qDebug() << "Function has been deleted!";
			setFunction(KNoID);
		}
	}
}

void VCButton::releaseFunction()
{
	Q_ASSERT(m_keyBind != NULL);

	if (m_function == KNoID)
	{
		return;
	}
	else if (m_keyBind->action() == KeyBind::Flash)
	{
		Function* function = _app->doc()->function(m_function);
		if (function != NULL && isOn() == true)
			function->stop();
	}
}

void VCButton::slotFlashReady()
{
	// This function is called twice with same XOR mask,
	// thus creating a brief opposite-color -- normal-color flash
	QPalette pal = palette();
	QColor color(pal.color(QPalette::Window));
	color.setRgb(color.red()^0xff, color.green()^0xff, color.blue()^0xff);
	pal.setColor(QPalette::Window, color);
	setPalette(pal);
}

/*****************************************************************************
 * Widget menu
 *****************************************************************************/

void VCButton::invokeMenu(QPoint point)
{
	QAction* attachAction;
	QMenu* menu;

	/* First, create the common widget menu and insert a separator to it */
	menu = VCWidget::createMenu();
	menu->addSeparator();

	/* Insert a menu item for attaching a function to this button */
	attachAction = menu->addAction(QIcon(":/attach.png"),
				       "Set function...", this,
				       SLOT(slotAttachFunction()));

	menu->exec(point);
	delete menu;
}

void VCButton::slotAttachFunction()
{
	FunctionSelection sel(this, false, KNoID, Function::Undefined, false);
	if (sel.exec() == QDialog::Accepted)
		setFunction(sel.selection.at(0));
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCButton::paintEvent(QPaintEvent* e)
{
	VCWidget::paintEvent(e);

	if (m_keyBind->action() == KeyBind::Flash)
	{
		/* TODO: Something better for marking a flash button */
		QPainter p(this);

		QPen pen(Qt::red);
		p.setPen(pen);
		p.drawEllipse(rect().width() - 14, rect().height() - 14,
			      10, 10);
	}
}

void VCButton::mousePressEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
		VCWidget::mousePressEvent(e);
	else
		pressFunction();
}

void VCButton::mouseReleaseEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
		VCWidget::mouseReleaseEvent(e);
	else
		releaseFunction();
}
