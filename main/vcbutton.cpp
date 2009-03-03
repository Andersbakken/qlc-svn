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

#include <QStyleOptionButton>
#include <QColorDialog>
#include <QFileDialog>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>
#include <QString>
#include <QDebug>
#include <QEvent>
#include <QTimer>
#include <QBrush>
#include <QStyle>
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
#include "fixture.h"
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

	setCaption(QString::null);
	setOn(false);
	setExclusive(false);
	setStopFunctions(false);
	setFrameStyle(KVCFrameStyleNone);

	/* Menu actions */
	m_chooseIconAction = new QAction(QIcon(":/image.png"), tr("Choose..."),
					 this);
	m_resetIconAction = new QAction(QIcon(":/undo.png"), tr("None"), this);	
	connect(m_chooseIconAction, SIGNAL(triggered(bool)),
		this, SLOT(slotChooseIcon()));
	connect(m_resetIconAction, SIGNAL(triggered(bool)),
		this, SLOT(slotResetIcon()));

	/* Initial size */
	setMinimumSize(20, 20);
	resize(QPoint(30, 30));

	setStyle(App::saneStyle());
	
	/* Listen to function removals */
	connect(_app->doc(), SIGNAL(functionRemoved(t_function_id)),
		this, SLOT(slotFunctionRemoved(t_function_id)));
}

VCButton::~VCButton()
{
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCButton::createCopy(VCWidget* parent)
{
	Q_ASSERT(parent != NULL);

	VCButton* button = new VCButton(parent);
	if (button->copyFrom(this) == false)
	{
		delete button;
		button = NULL;
	}

	return button;
}

bool VCButton::copyFrom(VCWidget* widget)
{
	VCButton* button = qobject_cast <VCButton*> (widget);
	if (button == NULL)
		return false;

	/* Copy button-specific stuff */
	setIcon(button->icon());
	setKeySequence(button->keySequence());
	setFunction(button->function());
	setExclusive(button->isExclusive());
	setStopFunctions(button->stopFunctions());
	
	/* Copy common stuff */
	return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCButton::editProperties()
{
	VCButtonProperties prop(this, _app);
	if (prop.exec() == QDialog::Accepted)
		_app->doc()->setModified();
}

/*****************************************************************************
 * Background color
 *****************************************************************************/

void VCButton::setBackgroundColor(const QColor& color)
{
	QPalette pal = palette();

	m_hasCustomBackgroundColor = true;
	m_backgroundImage = QString::null;
	pal.setColor(QPalette::Button, color);
	setPalette(pal);

	_app->doc()->setModified();
}

void VCButton::resetBackgroundColor()
{
	QColor fg;

	m_hasCustomBackgroundColor = false;
	m_backgroundImage = QString::null;

	/* Store foreground color */
	if (m_hasCustomForegroundColor == true)
		fg = palette().color(QPalette::ButtonText);

	/* Reset the whole palette to application palette */
	setPalette(QApplication::palette());

	/* Restore foreground color */
	if (fg.isValid() == true)
	{
		QPalette pal = palette();
		pal.setColor(QPalette::ButtonText, fg);
		setPalette(pal);
	}

	_app->doc()->setModified();
}

/*****************************************************************************
 * Foreground color
 *****************************************************************************/

void VCButton::setForegroundColor(const QColor& color)
{
	QPalette pal = palette();

	m_hasCustomForegroundColor = true;

	pal.setColor(QPalette::ButtonText, color);
	setPalette(pal);

	_app->doc()->setModified();
}

void VCButton::resetForegroundColor()
{
	QColor bg;

	m_hasCustomForegroundColor = false;

	/* Store background color */
	if (m_hasCustomBackgroundColor == true)
		bg = palette().color(QPalette::Button);

	/* Reset the whole palette to application palette */
	setPalette(QApplication::palette());

	/* Restore foreground color */
	if (bg.isValid() == true)
		setBackgroundColor(bg);
	else if (m_backgroundImage.isEmpty() == false)
		setBackgroundImage(m_backgroundImage);

	_app->doc()->setModified();
}

/*****************************************************************************
 * Button icon
 *****************************************************************************/

void VCButton::setIcon(const QString& icon)
{
	m_icon = icon;
	update();
}

void VCButton::slotChooseIcon()
{
	/* No point coming here if there is no VC */
	VirtualConsole* vc = VirtualConsole::instance();
	if (vc == NULL)
		return;

	QString path;
	path = QFileDialog::getOpenFileName(this, tr("Select button icon"),
			icon(), tr("Images (*.png *.xpm *.jpg *.gif)"));
        if (path.isEmpty() == false)
	{
		VCWidget* widget;
		foreach(widget, vc->selectedWidgets())
		{
			VCButton* button = qobject_cast<VCButton*> (widget);
			if (button != NULL)
				button->setIcon(path);
		}
	}
}

void VCButton::slotResetIcon()
{
	m_icon = QString::null;
	update();
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

	/* Icon */
	setIcon(root->attribute(KXMLQLCVCButtonIcon));

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
#warning LOAD KEY SEQUENCE
/*		else if (tag.tagName() == KXMLQLCKeyBind)
		{
			m_keyBind.loadXML(doc, &tag);
		}
*/
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

	/* VC button entry */
	root = doc->createElement(KXMLQLCVCButton);
	vc_root->appendChild(root);

	/* Caption */
	root.setAttribute(KXMLQLCVCCaption, caption());

	/* Icon */
	root.setAttribute(KXMLQLCVCButtonIcon, icon());

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
#warning SAVE KEY SEQUENCE
	//m_keyBind.saveXML(doc, &root);

	return true;
}

/*****************************************************************************
 * Button state
 *****************************************************************************/

void VCButton::setOn(bool on)
{
	m_on = on;

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

	update();
}

/*****************************************************************************
 * Key sequence handler
 *****************************************************************************/

void VCButton::setKeySequence(const QKeySequence& keySequence)
{
	m_keySequence = QKeySequence(keySequence);
}

void VCButton::slotKeyPressed(const QKeySequence& keySequence)
{
	if (m_keySequence == keySequence)
		pressFunction();
}

void VCButton::slotKeyReleased(const QKeySequence& keySequence)
{
	if (m_keySequence == keySequence)
		releaseFunction();
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VCButton::slotInputValueChanged(t_input_universe universe,
				     t_input_channel channel,
				     t_input_value value)
{
	Q_UNUSED(value);

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

void VCButton::slotFunctionRemoved(t_function_id fid)
{
	/* Invalidate the button's function if it's the one that was removed */
	if (fid == m_function)
		setFunction(KNoID);
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

#warning BUTTON ACTIONS

	/* TODO: Should this return immediately? */
	if (m_stopFunctions == true)
		_app->slotControlPanic();

	if (m_function == KNoID)
	{
		return;
	}
	else if (/*m_keyBind.action() == KeyBind::Toggle &&*/
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
	else if (/*m_keyBind.action() == KeyBind::Toggle &&*/
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
	else if ( 0 ) //m_keyBind.action() == KeyBind::Flash)
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

#warning BUTTON ACTIONS
/*
	else if (m_keyBind.action() == KeyBind::Flash)
	{
		Function* function = _app->doc()->function(m_function);
		if (function != NULL && isOn() == true)
			function->stop();
	}
*/
}

void VCButton::slotFlashReady()
{
	// This function is called twice with same XOR mask,
	// thus creating a brief opposite-color -- normal-color flash
	QPalette pal = palette();
	QColor color(pal.color(QPalette::Button));
	color.setRgb(color.red()^0xff, color.green()^0xff, color.blue()^0xff);
	pal.setColor(QPalette::Button, color);
	setPalette(pal);
}

/*****************************************************************************
 * Custom menu
 *****************************************************************************/

QMenu* VCButton::customMenu(QMenu* parentMenu)
{
	QMenu* menu = new QMenu(parentMenu);
	menu->setTitle(tr("Icon"));
	menu->addAction(m_chooseIconAction);
	menu->addAction(m_resetIconAction);

	return menu;
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCButton::paintEvent(QPaintEvent* e)
{
	QStyleOptionButton option;
	option.initFrom(this);

	/* Caption */
	option.text = caption();

	/* Sunken or raised based on isOn() status */
	if (isOn() == true)
		option.state = QStyle::State_Sunken;
	else
		option.state= QStyle::State_Raised;

	/* Enabled or disabled looks based on application mode */
	if (_app->mode() == App::Operate)
		option.state |= QStyle::State_Enabled;

	/* Icon */
	if (icon() != QString::null)
	{
		option.icon = QIcon(icon());
		option.iconSize = QSize(26, 26);
	}
	else
	{
		option.icon = QIcon();
		option.iconSize = QSize(-1, -1);
	}

	/* Paint the button */
	QPainter painter(this);
	style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);

	/* Flash emblem */
#warning BUTTON ACTIONS
/*
	if (m_keyBind.action() == KeyBind::Flash)
	{
		QIcon icon(":/flash.png");
		painter.drawPixmap(rect().width() - 16, 0,
			icon.pixmap(QSize(16, 16), QIcon::Normal, QIcon::On));
	}
*/
	/* Stop painting here */
	painter.end();

	/* Draw a selection frame if appropriate */
	VCWidget::paintEvent(e);
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
