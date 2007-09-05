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
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qevent.h>
#include <qspinbox.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qcursor.h>
#include <qtooltip.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qfontdialog.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <limits.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>
#include <qobjectlist.h>
#include <assert.h>

#include "common/settings.h"
#include "common/minmax.h"
#include "vcbutton.h"
#include "app.h"
#include "doc.h"
#include "function.h"
#include "floatingedit.h"
#include "vcbuttonproperties.h"
#include "vcframe.h"
#include "virtualconsole.h"
#include "keybind.h"
#include "configkeys.h"

extern App* _app;

const int KColorMask      ( 0xff ); // Produces opposite colors with XOR
const int KFlashReadyTime (   50 ); // 1/4 second

VCButton::VCButton(QWidget* parent) :
        QPushButton   ( parent, "Button" ),
        m_functionID  ( KNoID ),
        m_resizeMode  ( false ),
        m_keyBind     ( NULL ),
        m_isExclusive ( false )
{
}

VCButton::~VCButton()
{
}

void VCButton::init()
{
	setToggleButton(true);
	m_channel = 0;
	m_stopFunctions = FALSE;

	assert(m_keyBind == NULL);
	m_keyBind = new KeyBind();

	connect(m_keyBind, SIGNAL(pressed()), this, SLOT(pressFunction()));
	connect(m_keyBind, SIGNAL(released()), this, SLOT(releaseFunction()));

	connect(_app->virtualConsole(), SIGNAL(sendFeedBack()), this, SLOT(slotFeedBack()));
	connect(_app->virtualConsole(), SIGNAL(InpEvent(const int, const int, const int)),
		this, SLOT(slotInputEvent(const int, const int, const int)));

	QToolTip::add(this, "No function");

	setMinimumSize(20, 20);
	resize(30, 30);

	connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
}

void VCButton::copyFrom(VCButton* button)
{
	attachFunction(button->m_functionID);

	m_resizeMode = false;

	assert(button->keyBind());
	if (m_keyBind)
	{
		disconnect(m_keyBind);
		delete m_keyBind;
	}

	m_keyBind = new KeyBind(button->keyBind());
	connect(m_keyBind, SIGNAL(pressed()), this, SLOT(pressFunction()));
	connect(m_keyBind, SIGNAL(released()), this, SLOT(releaseFunction()));

	setToggleButton(true);

	setCaption(button->caption());

	if (button->ownFont())
	{
		setFont(button->font());
	}

	if (button->ownPalette())
	{
		setPaletteForegroundColor(button->paletteForegroundColor());
		setPaletteBackgroundColor(button->paletteBackgroundColor());
	}

	if (button->paletteBackgroundPixmap())
	{
		setPaletteBackgroundPixmap(*button->paletteBackgroundPixmap());
	}

	reparent(button->parentWidget(), 0, QPoint(0, 0), true);

	setGeometry(button->geometry());

	move(button->x() + button->width(), button->y());
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
	button->init();
	button->show();

	/* Continue loading */
	return button->loadXML(doc, root);
}

bool VCButton::loadXML(QDomDocument* doc, QDomElement* root)
{
}

bool VCButton::saveXML(QDomDocument* doc, QDomElement* root)
{
}

/*
void VCButton::saveToFile(QFile& file, unsigned int parentID)
{
	QString s;
	QString t;

	// Comment
	s = QString("# Virtual Console Button Entry\n");
	file.writeBlock((const char*) s, s.length());

	// Entry type
	s = QString("Entry = Button") + QString("\n");
	file.writeBlock((const char*) s, s.length());

	// Name
	s = QString("Name = ") + caption() + QString("\n");
	file.writeBlock((const char*) s, s.length());

	// Parent ID
	t.setNum(parentID);
	s = QString("Parent = ") + t + QString("\n");
	file.writeBlock((const char*) s, s.length());

	// X
	t.setNum(x());
	s = QString("X = ") + t + QString("\n");
	file.writeBlock((const char*) s, s.length());

	// Y
	t.setNum(y());
	s = QString("Y = ") + t + QString("\n");
	file.writeBlock((const char*) s, s.length());

	// W
	t.setNum(width());
	s = QString("Width = ") + t + QString("\n");
	file.writeBlock((const char*) s, s.length());

	// H
	t.setNum(height());
	s = QString("Height = ") + t + QString("\n");
	file.writeBlock((const char*) s, s.length());

	// Text color
	if (ownPalette())
	{
		t.setNum(qRgb(paletteForegroundColor().red(),
			      paletteForegroundColor().green(),
			      paletteForegroundColor().blue()));
		s = QString("Textcolor = ") + t + QString("\n");
		file.writeBlock((const char*) s, s.length());

		// Background color
		t.setNum(qRgb(paletteBackgroundColor().red(),
			      paletteBackgroundColor().green(),
			      paletteBackgroundColor().blue()));
		s = QString("Backgroundcolor = ") + t + QString("\n");
		file.writeBlock((const char*) s, s.length());
	}

	// Background pixmap
	if (paletteBackgroundPixmap())
	{
		s = QString("Pixmap = " + iconText() + QString("\n"));
		file.writeBlock((const char*) s, s.length());
	}

	// Font
	s = QString("Font = ") + font().toString() + QString("\n");
	file.writeBlock((const char*) s, s.length());

	// Function
	s.sprintf("Function = %d\n", m_functionID);
	file.writeBlock((const char*) s, s.length());

	// Key binding
	assert(m_keyBind);

	s.sprintf("BindKey = %d\n", m_keyBind->key());
	file.writeBlock((const char*) s, s.length());

	s.sprintf("BindMod = %d\n", m_keyBind->mod());
	file.writeBlock((const char*) s, s.length());

	s.sprintf("BindPress = %d\n", m_keyBind->pressAction());
	file.writeBlock((const char*) s, s.length());

	s.sprintf("BindRelease = %d\n", m_keyBind->releaseAction());
	file.writeBlock((const char*) s, s.length());

	// Midi stuff
	t.setNum(m_channel);
	s = QString("InputChannel = ") + t + QString("\n");
	file.writeBlock((const char*) s, s.length());

	// Panicbutton
	if( stopFunctions() )
	{
		s = QString("StopFunctions = TRUE") + QString("\n");
		file.writeBlock((const char*) s, s.length());
	}
}
*/

/*********************************************************************
 * 
 *********************************************************************/
void VCButton::setCaption(const QString& text)
{
	setText(text);
	QWidget::setCaption(text);
}

void VCButton::setKeyBind(const KeyBind* kb)
{
	assert(kb);

	if (m_keyBind)
	{
		delete m_keyBind;
	}

	m_keyBind = new KeyBind(kb);

	connect(m_keyBind, SIGNAL(pressed()), this, SLOT(pressFunction()));
	connect(m_keyBind, SIGNAL(released()), this, SLOT(releaseFunction()));
}

// sure, we have made this over the frame stuff
// but it works for the moment and can change sometime
void VCButton::setExclusive(bool exclusive)
{
	m_isExclusive = exclusive;
}

void VCButton::mousePressEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		_app->virtualConsole()->setSelectedWidget(this);

		if (m_resizeMode == true)
		{
			setMouseTracking(false);
			m_resizeMode = false;
		}

		if (e->button() & MidButton || e->button() & LeftButton)
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
		pressFunction();
	}
}


void VCButton::invokeMenu(QPoint point)
{
	_app->virtualConsole()->editMenu()->exec(point);
}

void VCButton::parseWidgetMenu(int item)
{
	switch (item)
	{
	case KVCMenuEditProperties:
        {
		VCButtonProperties* p = NULL;
		p = new VCButtonProperties(this);
		p->initView();
		p->exec();
		delete p;
        }
        break;

	case KVCMenuEditCopy:
        {
		VCButton* bt = NULL;
		bt = new VCButton(parentWidget());
		bt->init();
		bt->copyFrom(this);
		bt->show();

		_app->doc()->setModified();
        }
        break;

	default:
		break;
	}
}

void VCButton::mouseReleaseEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		unsetCursor();
		m_resizeMode = false;
		setMouseTracking(false);
	}
	else
	{
		releaseFunction();
	}
}

void VCButton::mouseMoveEvent(QMouseEvent* e)
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
		QPushButton::mouseMoveEvent(e);
	}
}

void VCButton::resizeTo(QPoint p)
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


void VCButton::moveTo(QPoint p)
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


void VCButton::mouseDoubleClickEvent(QMouseEvent* e)
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

void VCButton::paintEvent(QPaintEvent* e)
{
	QPushButton::paintEvent(e);

	if (_app->mode() == App::Design &&
            _app->virtualConsole()->selectedWidget() == this)
	{
		QPainter p(this);

		// Draw a dotted line around the widget
		QPen pen(DotLine);
		pen.setWidth(2);
		p.setPen(pen);
		p.drawRect(1, 1, rect().width() - 1, rect().height() - 1);

		// Draw a resize handle
		QBrush b(SolidPattern);
		p.fillRect(rect().width() - 10, rect().height() - 10, 10, 10, b);
	}

	else if (m_keyBind->pressAction() == KeyBind::PressFlash)
	{
		QPainter p(this);

		QPen pen(red);
		p.setPen(pen);
		p.drawEllipse(rect().width() - 14, rect().height() - 14, 10, 10);
	}
}


void VCButton::slotModeChanged()
{
	repaint();
}


void VCButton::pressFunction()
{
	Function* f = NULL;

	assert(m_keyBind);


	if( m_stopFunctions )
	{
		_app->slotPanic();
	}

	if (m_functionID == KNoID)
	{
		return;
	}
	else if (m_keyBind->pressAction() == KeyBind::PressToggle &&
		 m_isExclusive == false)
	{
		f = _app->doc()->function(m_functionID);
		if (f)
		{
			if (isOn())
			{
				f->stop();
			}
			else
			{
				if (f->engage(static_cast<QObject*> (this)))
				{
					setOn(true);
				}
			}
		}
		else
		{
			qDebug("Function has been deleted!");
			attachFunction(KNoID);
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
			if (((VCButton*)obj)->isOn())
			{
				f = _app->doc()->function(((VCButton*)obj)->functionID());
				f->stop();
			}
		}
		delete l;

		f = _app->doc()->function(m_functionID);
		if (f)
		{
			if (f->engage(static_cast<QObject*> (this)))
			{
				setOn(true);
			}
		}
		else
		{
			qDebug("Function has been deleted!");
			attachFunction(KNoID);
		}
	}
	else if (m_keyBind->pressAction() == KeyBind::PressFlash)
	{
		f = _app->doc()->function(m_functionID);
		if (f)
		{
			if (f->engage(static_cast<QObject*> (this)))
			{
				setOn(true);
			}
		}
		else
		{
			qDebug("Function has been deleted!");
			attachFunction(KNoID);
		}
	}
}

void VCButton::releaseFunction()
{
	Function* f = NULL;

	assert(m_keyBind);

	if (m_functionID == KNoID)
	{
		return;
	}
	else if (m_keyBind->releaseAction() == KeyBind::ReleaseNothing)
	{
		return;
	}
	else if (m_keyBind->releaseAction() == KeyBind::ReleaseStop)
	{
		f = _app->doc()->function(m_functionID);
		if (f)
		{
			if (isOn())
			{
				f->stop();
			}
		}
	}
}

void VCButton::attachFunction(t_function_id id)
{
	m_functionID = id;

	Function* f = _app->doc()->function(id);
	if (f)
	{
		QToolTip::add(this, f->name());
	}
	else
	{
		QToolTip::add(this, "No function");
	}

	_app->doc()->setModified();
}

void VCButton::customEvent(QCustomEvent* e)
{
	if (e->type() == KFunctionStopEvent &&
            ((FunctionStopEvent*)e)->functionID() == m_functionID)
	{
		setOn(false);
		slotFlashReady();
		QTimer::singleShot(KFlashReadyTime, this, SLOT(slotFlashReady()));
	}
	else if (e->type() == KVCMenuEvent)
	{
		parseWidgetMenu(((VCMenuEvent*) e)->menuItem());
	}
}

void VCButton::slotFlashReady()
{
	//
	// This function is called twice with same XOR mask,
	// thus creating a brief opposite-color-normal-color flash
	//
	QColor c(backgroundColor());
	c.setRgb(c.red() ^ KColorMask,
		 c.green() ^ KColorMask,
		 c.blue() ^ KColorMask);
	setPaletteBackgroundColor(c);
}

void VCButton::slotInputEvent(const int id, const int channel, const int value)
{
	QString t;
	//t.sprintf("Virtual Console Slider: InputEvent  %d  %d  %d", id, channel, value);
	//qDebug(t);
	if( id ==0 && channel == m_channel)
	{
		pressFunction();
	}
	//slotBusValueChanged(m_busID, value);
}



void VCButton::slotFeedBack()
{
	/*    int value = m_slider->value();
	      if (m_mode == Speed)
	      {
      
	      int range = m_busHighLimit - m_busLowLimit;//m_levelHighLimit - m_levelLowLimit;
	      //m_slider->setValue(m_busHighLimit* KFrequency - (range*value* KFrequency)/127 );
	      float f = ((float) value / (float) KFrequency );
	      //t.sprintf("%f", f);
	      //qDebug(t);
	      _app->inputPlugin()->feedBack( 1,m_channel, 127 - int( (f * 127) / range));
	      } 
	      else if (m_mode == Level)
	      {
	      _app->inputPlugin()->feedBack( 1, m_channel, 127 - (value*127)/255);
	      }
	      else
	      {
	      _app->inputPlugin()->feedBack( 1, m_channel, 127 - (value*127) / 100);
	      }
	*/
}


