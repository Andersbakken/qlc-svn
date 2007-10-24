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
#include <qinputdialog.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <limits.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>
#include <qobjectlist.h>
#include <assert.h>

#include "common/filehandler.h"
#include "common/qlcimagepreview.h"

#include "vcbutton.h"
#include "app.h"
#include "doc.h"
#include "function.h"
#include "vcbuttonproperties.h"
#include "vcframe.h"
#include "virtualconsole.h"
#include "keybind.h"

extern App* _app;

const int KColorMask      ( 0xff ); // Produces opposite colors with XOR
const int KFlashReadyTime (   50 ); // 1/4 second

VCButton::VCButton(QWidget* parent) : QPushButton(parent, "Button")
{
	m_hasCustomForegroundColor = false;
	m_hasCustomBackgroundColor = false;
	m_hasCustomFont = false;

	m_keyBind = NULL;

	m_functionID = KNoID;
	m_isExclusive = false;

	m_inputChannel = 0;
	m_stopFunctions = false;

	m_mousePressPoint = QPoint();
	m_resizeMode = false;
}

VCButton::~VCButton()
{
	if (m_keyBind != NULL)
		delete m_keyBind;
}

void VCButton::init()
{
	setToggleButton(true);
	setInputChannel(0);
	m_stopFunctions = FALSE;
	setCaption("");

	setMinimumSize(20, 20);
	resize(QPoint(30, 30));

	m_keyBind = new KeyBind();

	connect(m_keyBind, SIGNAL(pressed()), this, SLOT(pressFunction()));
	connect(m_keyBind, SIGNAL(released()), this, SLOT(releaseFunction()));

	connect(_app->virtualConsole(), SIGNAL(sendFeedBack()),
		this, SLOT(slotFeedBack()));
	connect(_app->virtualConsole(), SIGNAL(InpEvent(const int, const int, const int)),
		this, SLOT(slotInputEvent(const int, const int, const int)));

	QToolTip::add(this, "No function");

	connect(_app, SIGNAL(modeChanged(App::Mode)),
		this, SLOT(slotModeChanged(App::Mode)));
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

/*********************************************************************
 * Background image
 *********************************************************************/
void VCButton::setBackgroundImage(const QString& path)
{
	m_hasCustomBackgroundColor = false;
	m_backgroundImage = path;
	setIconText(path);
	setPaletteBackgroundPixmap(QPixmap(path));
	_app->doc()->setModified();
}

const QString& VCButton::backgroundImage()
{
	return m_backgroundImage;
}

void VCButton::chooseBackgroundImage()
{
	QLCImagePreview* preview = new QLCImagePreview();
	QFileDialog* fd = new QFileDialog(this);
	fd->setContentsPreviewEnabled(true);
	fd->setContentsPreview(preview, preview);
	fd->setPreviewMode(QFileDialog::Contents);
	fd->setFilter("Images (*.png *.xpm *.jpg *.gif)");
	fd->setSelection(backgroundImage());
	
	if (fd->exec() == QDialog::Accepted)
		setBackgroundImage(fd->selectedFile());
	
	delete preview;
	delete fd;
}

/*********************************************************************
 * Background color
 *********************************************************************/
void VCButton::setBackgroundColor(const QColor& color)
{
	m_hasCustomBackgroundColor = true;
	m_backgroundImage = QString::null;
	setPaletteBackgroundColor(color);
	_app->doc()->setModified();
}

void VCButton::resetBackgroundColor()
{
	QColor fg;

	m_hasCustomBackgroundColor = false;
	m_backgroundImage = QString::null;

	/* Store foreground color */
	if (m_hasCustomForegroundColor == true)
		fg = paletteForegroundColor();

	/* Reset the whole palette */
	unsetPalette();

	/* Restore foreground color */
	if (fg.isValid() == true)
		setPaletteForegroundColor(fg);

	_app->doc()->setModified();
}

void VCButton::chooseBackgroundColor()
{
	QColor color;
	color = QColorDialog::getColor(backgroundColor());
	if (color.isValid())
		setBackgroundColor(color);
}

/*********************************************************************
 * Foreground color
 *********************************************************************/
void VCButton::setForegroundColor(const QColor& color)
{
	m_hasCustomForegroundColor = true;
	setPaletteForegroundColor(color);
	_app->doc()->setModified();
}

void VCButton::resetForegroundColor()
{
	QColor bg;

	m_hasCustomForegroundColor = false;

	/* Store background color */
	if (m_hasCustomBackgroundColor == true)
		bg = paletteBackgroundColor();

	/* Reset the whole palette */
	unsetPalette();

	/* Restore foreground color */
	if (bg.isValid() == true)
		setPaletteBackgroundColor(bg);
	else if (m_backgroundImage.isEmpty() == false)
		setPaletteBackgroundPixmap(QPixmap(m_backgroundImage));

	_app->doc()->setModified();
}

void VCButton::chooseForegroundColor()
{
	QColor color;
	color = QColorDialog::getColor(foregroundColor());
	if (color.isValid())
		setForegroundColor(color);
}

/*********************************************************************
 * Font
 *********************************************************************/

void VCButton::setFont(const QFont& font)
{
	m_hasCustomFont = true;
	QWidget::setFont(font);
	_app->doc()->setModified();
}

void VCButton::resetFont()
{
	m_hasCustomFont = false;
	unsetFont();
	_app->doc()->setModified();
}

void VCButton::chooseFont()
{
	bool ok = false;
	QFont f = QFontDialog::getFont(&ok, font());
	if (ok == true)
		setFont(f);
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCButton::setCaption(const QString& text)
{
	setText(text);
	QWidget::setCaption(text);
	_app->doc()->setModified();
}

void VCButton::rename()
{
	QString text;
	bool ok = false;

	text = QInputDialog::getText("Rename button",
				     "Set button caption:",
				     QLineEdit::Normal,
				     QString::null, &ok, this);
	
	if (ok == true && text.isEmpty() == false)
		setCaption(text);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/
void VCButton::editProperties()
{
	VCButtonProperties* prop = NULL;

	prop = new VCButtonProperties(this, _app);
	prop->initView();

	if (prop->exec() == QDialog::Accepted)
		_app->doc()->setModified();

	delete prop;
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
			attachFunction(str.toInt());
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
	str.setNum(functionID());
	tag.setAttribute(KXMLQLCVCButtonFunctionID, str);

	/* Midi input */
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

/*
  void VCButton::saveToFile(QFile& file, unsigned int parentID)
  {
  // Panicbutton
  if (stopFunctions() == true)
  {
  s = QString("StopFunctions = TRUE") + QString("\n");
  file.writeBlock((const char*) s, s.length());
  }
  }
*/

bool VCButton::loadXMLAppearance(QDomDocument* doc, QDomElement* root)
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
		if (tag.tagName() == KXMLQLCVCForegroundColor)
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

bool VCButton::saveXMLAppearance(QDomDocument* doc, QDomElement* btn_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(btn_root != NULL);

	/* VC Label entry */
	root = doc->createElement(KXMLQLCVCAppearance);
	btn_root->appendChild(root);

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

/*****************************************************************************
 * QLC Mode change
 *****************************************************************************/

void VCButton::slotModeChanged(App::Mode mode)
{
	repaint();
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
 * Widget move & resize
 *****************************************************************************/

void VCButton::resize(QPoint p)
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
	QButton::resize(p.x(), p.y());
}

void VCButton::move(QPoint p)
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
	QButton::move(p);
}

/*****************************************************************************
 * Widget menu
 *****************************************************************************/

void VCButton::invokeMenu(QPoint point)
{
	// Foreground menu
	QPopupMenu* fgMenu = new QPopupMenu();
	fgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/color.png")),
			   "&Color...", KVCMenuForegroundColor);
	fgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", KVCMenuForegroundDefault);

	// Background menu
	QPopupMenu* bgMenu = new QPopupMenu();
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/color.png")),
			   "&Color...", KVCMenuBackgroundColor);
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/image.png")),
			   "&Image...", KVCMenuBackgroundImage);
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", KVCMenuBackgroundDefault);

	// Font menu
	QPopupMenu* fontMenu = new QPopupMenu();
	fontMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/fonts.png")),
			     "&Font...", KVCMenuFont);
	fontMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", KVCMenuFontDefault);

	// Stacking order menu
	QPopupMenu* stackMenu = new QPopupMenu();
	stackMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/up.png")),
			      "Bring to &Front", KVCMenuStackingRaise);
	stackMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/down.png")),
			      "Send to &Back", KVCMenuStackingLower);

	// Edit menu
	QPopupMenu* editMenu = new QPopupMenu();
	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editcut.png")),
			     "Cut", KVCMenuEditCut);

	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editcopy.png")),
			     "Copy", KVCMenuEditCopy);

	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editpaste.png")),
			     "Paste", KVCMenuEditPaste);

	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editdelete.png")),
			     "Delete", KVCMenuEditDelete);

	editMenu->insertSeparator();

	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editclear.png")),
			       "&Rename...", KVCMenuEditRename);

	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/configure.png")),
			       "&Properties...", KVCMenuEditProperties);

	editMenu->setItemEnabled(KVCMenuEditCut, false);
	editMenu->setItemEnabled(KVCMenuEditCopy, false);
	editMenu->setItemEnabled(KVCMenuEditPaste, false);

	editMenu->insertSeparator();

	editMenu->insertItem("Background", bgMenu, 0);
	editMenu->insertItem("Foreground", fgMenu, 0);
	editMenu->insertItem("Font", fontMenu, 0);
	editMenu->insertItem("Stacking Order", stackMenu, 0);

	connect(editMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(bgMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(fgMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(fontMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(stackMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	
	editMenu->exec(point);
	delete editMenu;
	delete bgMenu;
	delete fgMenu;
	delete fontMenu;
	delete stackMenu;
}

void VCButton::slotMenuCallback(int item)
{
	switch (item)
	{
	case KVCMenuEditCut:
		break;
	case KVCMenuEditCopy:
		break;
	case KVCMenuEditPaste:
		break;
	case KVCMenuEditDelete:
		scram();
		break;

	case KVCMenuEditRename:
		rename();
		break;

	case KVCMenuEditProperties:
		editProperties();
		break;

	case KVCMenuForegroundColor:
		chooseForegroundColor();
		break;

	case KVCMenuForegroundDefault:
		resetForegroundColor();
		break;

	case KVCMenuBackgroundColor:
		chooseBackgroundColor();
		break;
	
	case KVCMenuBackgroundImage:
		chooseBackgroundImage();
		break;
	
	case KVCMenuBackgroundDefault:
		resetBackgroundColor();
		break;

	case KVCMenuFont:
		chooseFont();
		break;

	case KVCMenuFontDefault:
		resetFont();
		break;

	case KVCMenuStackingRaise:
		raise();
		break;

	case KVCMenuStackingLower:
		lower();
		break;

	default:
		break;
	}
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

	if (m_functionID == KNoID)
	{
		return;
	}
	else if (m_keyBind->pressAction() == KeyBind::PressToggle &&
		 m_isExclusive == false)
	{
		f = _app->doc()->function(m_functionID);
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
			if (((VCButton*)obj)->isOn() == true)
			{
				f = _app->doc()->function(
					((VCButton*)obj)->functionID());
				f->stop();
			}
		}
		delete l;

		f = _app->doc()->function(m_functionID);
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
			attachFunction(KNoID);
		}
	}
	else if (m_keyBind->pressAction() == KeyBind::PressFlash)
	{
		f = _app->doc()->function(m_functionID);
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
			attachFunction(KNoID);
		}
	}
}

void VCButton::releaseFunction()
{
	Q_ASSERT(m_keyBind != NULL);

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
		Function* function = NULL;
		function = _app->doc()->function(m_functionID);
		if (function != NULL)
		{
			if (isOn() == true)
			{
				function->stop();
			}
		}
	}
}

void VCButton::attachFunction(t_function_id id)
{
	m_functionID = id;

	Function* function = _app->doc()->function(id);
	if (function != NULL)
		QToolTip::add(this, function->name());
	else
		QToolTip::add(this, "No function");

	_app->doc()->setModified();
}

void VCButton::setExclusive(bool exclusive)
{
	// sure, we have made this over the frame stuff
	// but it works for the moment and can change sometime
	m_isExclusive = exclusive;
}

/*****************************************************************************
 * Ready flash
 *****************************************************************************/

void VCButton::customEvent(QCustomEvent* e)
{
	if (e->type() == KFunctionStopEvent &&
            ((FunctionStopEvent*)e)->functionID() == m_functionID)
	{
		setOn(false);
		slotFlashReady();
		QTimer::singleShot(KFlashReadyTime, this, SLOT(slotFlashReady()));
	}
}

void VCButton::slotFlashReady()
{
	// This function is called twice with same XOR mask,
	// thus creating a brief opposite-color -- normal-color flash
	QColor c(backgroundColor());
	c.setRgb(c.red() ^ KColorMask,
		 c.green() ^ KColorMask,
		 c.blue() ^ KColorMask);
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

 /****************************************************************************
  * Event handlers
  ****************************************************************************/

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

void VCButton::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
		slotMenuCallback(KVCMenuEditProperties);
}

void VCButton::mouseMoveEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		if (m_resizeMode == true)
		{
			QPoint p(QCursor::pos());
			resize(mapFromGlobal(p));
			_app->doc()->setModified();
		}
		else if (e->state() & LeftButton || e->state() & MidButton)
		{
			QPoint p(parentWidget()->mapFromGlobal(QCursor::pos()));
			p.setX(p.x() - m_mousePressPoint.x());
			p.setY(p.y() - m_mousePressPoint.y());

			move(p);
			_app->doc()->setModified();
		}
	}
	else
	{
		QPushButton::mouseMoveEvent(e);
	}
}
