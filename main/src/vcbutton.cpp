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

#include <qdom.h>
#include <qevent.h>
#include <qstring.h>
#include <qcursor.h>
#include <qtooltip.h>
#include <limits.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>
#include <qobjectlist.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>

#include "common/filehandler.h"
#include "common/qlcimagepreview.h"

#include "app.h"
#include "doc.h"
#include "vcbutton.h"
#include "vcframe.h"
#include "function.h"
#include "fixture.h"
#include "keybind.h"
#include "virtualconsole.h"
#include "vcbuttonproperties.h"
#include "functionmanager.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCButton::VCButton(QWidget* parent) : VCWidget(parent, "Button")
{
	/* Just a pointer to the function selection dialog. This is used
	   only when the menu item for selecting a function is clicked */
	m_functionManager = NULL;

	/* No function is initially attached to the button */
	m_function = KNoID;
	QToolTip::add(this, "No function");

	/* Set the frame line width a bit thicker to make this look more
	   like a button than a frame (which it actually is) */
	setLineWidth(2);

	setCaption("");
	setOn(false);
	setExclusive(false);
	m_stopFunctions = false;

	/* Initial size */
	setMinimumSize(20, 20);
	resize(QPoint(30, 30));

	/* Keybinding */
	m_keyBind = new KeyBind();
	connect(m_keyBind, SIGNAL(pressed()), this, SLOT(pressFunction()));
	connect(m_keyBind, SIGNAL(released()), this, SLOT(releaseFunction()));

	/* External input stuff */
	setInputChannel(0);
	connect(_app->virtualConsole(),
		SIGNAL(sendFeedBack()),
		this,
		SLOT(slotFeedBack()));

	connect(_app->virtualConsole(),
		SIGNAL(InpEvent(const int, const int, const int)),
		this,
		SLOT(slotInputEvent(const int, const int, const int)));
}

VCButton::~VCButton()
{
	if (m_keyBind != NULL)
		delete m_keyBind;
	m_keyBind = NULL;
}

void VCButton::scram()
{
	int result = QMessageBox::warning(this,
					  QString(caption()),
					  QString("Remove selected button?"),
					  QMessageBox::Yes,
					  QMessageBox::No);
	
	if (result == QMessageBox::Yes)
	{
		_app->virtualConsole()->setSelectedWidget(NULL);
		_app->doc()->setModified();
		deleteLater();
	}
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCButton::editProperties()
{
	VCButtonProperties prop(this, _app);
	prop.initView();

	if (prop.exec() == QDialog::Accepted)
		_app->doc()->setModified();
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
		qWarning("Button node not found!");
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
		qWarning("Button node not found!");
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
		else if (tag.tagName() == KXMLQLCVCButtonFunction)
		{
			str = tag.attribute(KXMLQLCVCButtonFunctionID);
			setFunction(str.toInt());
		}
		else if (tag.tagName() == KXMLQLCVCButtonInputChannel)
		{
			setInputChannel(tag.text().toInt());
		}
		else if (tag.tagName() == KXMLQLCKeyBind)
		{
			if (m_keyBind != NULL)
				delete m_keyBind;
			m_keyBind = new KeyBind();
			m_keyBind->loadXML(doc, &tag);
		}
		else
		{
			qWarning("Unknown button tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

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

	/* Input */
	tag = doc->createElement(KXMLQLCVCButtonInputChannel);
	root.appendChild(tag);
	str.setNum(m_inputChannel);
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Window state */
	FileHandler::saveXMLWindowState(doc, &root, this);

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
		VCWidget::setFrameStyle(KFrameStyleSunken);
	else
		VCWidget::setFrameStyle(KFrameStyleRaised);
}

void VCButton::setFrameStyle(const int style)
{
	/* Don't allow frame style setting because we need to simulate
	   button-look with a custom frame style */
}

/*****************************************************************************
 * KeyBind
 *****************************************************************************/

void VCButton::setKeyBind(const KeyBind* kb)
{
	Q_ASSERT(kb != NULL);

	if (m_keyBind)
		delete m_keyBind;

	m_keyBind = new KeyBind(kb);

	connect(m_keyBind, SIGNAL(pressed()), this, SLOT(pressFunction()));
	connect(m_keyBind, SIGNAL(released()), this, SLOT(releaseFunction()));
}

/*****************************************************************************
 * Function attachment
 *****************************************************************************/

void VCButton::selectFunction()
{
	if (m_functionManager == NULL)
	{
		// Create a new function manager
		m_functionManager = new FunctionManager(
			this, FunctionManager::SelectionMode);

		connect(m_functionManager, SIGNAL(closed()),
			this, SLOT(slotFunctionManagerClosed()));

		// Initialize the function manager dialog
		m_functionManager->init();
	}
	
	m_functionManager->show();
}

void VCButton::slotFunctionManagerClosed()
{
	FunctionIDList list;
	FunctionIDList::iterator it;
	Function* function = NULL;

	Q_ASSERT(m_functionManager != NULL);

	if (m_functionManager->result() == QDialog::Accepted)
	{
		// Just get the first of the selected items
		m_functionManager->selection(list);
		it = list.begin();

		// Do the attachment only if something sensible was selected
		if (it != list.end())
			setFunction(*it);
	}

	delete m_functionManager;
	m_functionManager = NULL;
}

void VCButton::setFunction(t_function_id function)
{
	Function* f = NULL;

	f = _app->doc()->function(function);
	if (f != NULL)
	{
		m_function = function;
		QToolTip::add(this, f->name());
	}
	else
	{
		m_function = KNoID;
		QToolTip::add(this, "No function");
		qWarning("Unable to attach nonexistent function %d to button",
			 function);
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

void VCButton::pressFunction()
{
	Function* f = NULL;

	if (m_stopFunctions == true)
	{
		_app->slotPanic();
	}

	if (m_function == KNoID)
	{
		return;
	}
	else if (m_keyBind->pressAction() == KeyBind::PressToggle &&
		 m_isExclusive == false)
	{
		f = _app->doc()->function(m_function);
		if (f != NULL)
		{
			if (isOn() == true)
			{
				f->stop();
			}
			else
			{
				if (f->engage(static_cast<QObject*> (this)) == true)
				{
					setOn(true);
				}
			}
		}
		else
		{
			qDebug("Function has been deleted!");
			setFunction(KNoID);
		}
	}
	else if (m_keyBind->pressAction() == KeyBind::PressToggle &&
		 m_isExclusive == true)
	{
		QObjectList* l = parentWidget()->queryList("VCButton");
		QObjectListIt it(*l);
		QObject* obj;
		while ((obj = it.current()) != NULL)
		{
			++it;
			if (((VCButton*)obj)->isOn() == true)
			{
				f = _app->doc()->function(
					((VCButton*)obj)->function());
				Q_ASSERT(f != NULL);
				f->stop();
			}
		}
		delete l;

		f = _app->doc()->function(m_function);
		if (f != NULL)
		{
			if (f->engage(static_cast<QObject*> (this)) == true)
			{
				setOn(true);
			}
		}
		else
		{
			qDebug("Function has been deleted!");
			setFunction(KNoID);
		}
	}
	else if (m_keyBind->pressAction() == KeyBind::PressFlash)
	{
		f = _app->doc()->function(m_function);
		if (f != NULL)
		{
			if (f->engage(static_cast<QObject*> (this)) == true)
			{
				setOn(true);
			}
		}
		else
		{
			qDebug("Function has been deleted!");
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
	else if (m_keyBind->releaseAction() == KeyBind::ReleaseNothing)
	{
		return;
	}
	else if (m_keyBind->releaseAction() == KeyBind::ReleaseStop)
	{
		Function* function = NULL;
		function = _app->doc()->function(m_function);
		if (function != NULL)
		{
			if (isOn() == true)
			{
				function->stop();
			}
		}
	}
}

void VCButton::functionStopEvent(FunctionStopEvent* e)
{
	if (e && e->functionID() == m_function)
	{
		setOn(false);
		slotFlashReady();

		/* Flash the button for 100ms */
		QTimer::singleShot(100, this, SLOT(slotFlashReady()));
	}
}

void VCButton::slotFlashReady()
{
	// This function is called twice with same XOR mask,
	// thus creating a brief opposite-color -- normal-color flash
	QColor c(backgroundColor());
	c.setRgb(c.red() ^ 0xff, c.green() ^ 0xff, c.blue() ^ 0xff);
	setPaletteBackgroundColor(c);
}

/*****************************************************************************
 * External sliderboard input
 *****************************************************************************/

void VCButton::slotInputEvent(const int id, const int channel, const int value)
{
	if (id == 0 && channel == inputChannel())
		pressFunction();
}

void VCButton::slotFeedBack()
{
	/* TODO? */
}

/*****************************************************************************
 * Widget menu
 *****************************************************************************/

void VCButton::invokeMenu(QPoint point)
{
	QPopupMenu* menu = NULL;

	/* First, create the common widget menu and insert a separator to it */
	menu = VCWidget::createMenu();
	menu->insertSeparator();

	/* Insert a menu item for attaching a function to this button */
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/attach.png")),
			 "Set Function...", KVCMenuEditAttach);

	menu->exec(point);

	delete menu;
}

void VCButton::slotMenuCallback(int item)
{
	switch (item)
	{
	case KVCMenuEditAttach:
		selectFunction();
		break;

	default:
		VCWidget::slotMenuCallback(item);
		break;
	}
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCButton::paintEvent(QPaintEvent* e)
{
	VCWidget::paintEvent(e);

	if (m_keyBind->pressAction() == KeyBind::PressFlash)
	{
		/* TODO: Something better for marking a flash button */
		QPainter p(this);
		
		QPen pen(red);
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

void VCButton::customEvent(QCustomEvent* e)
{
	if (e->type() == KFunctionStopEvent)
		functionStopEvent(static_cast<FunctionStopEvent*> (e));
}

