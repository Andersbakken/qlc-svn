/*
  Q Light Controller
  virtualconsole.h
  
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

#ifndef VIRTUALCONSOLE_H
#define VIRTUALCONSOLE_H

#include <qwidget.h>
#include <qframe.h>
#include <qptrlist.h>

class QMenuBar;
class QPopupMenu;
class QToolBar;
class QHBoxLayout;
class QFile;
class QFrame;
class QDomDocument;
class QDomElement;

class VCFrame;
class KeyBind;
class Bus;
class VCDockArea;
class FloatingEdit;

#define KXMLQLCVirtualConsole "VirtualConsole"

#define KXMLQLCVCCaption "Caption"

#define KXMLQLCVirtualConsoleGrid "Grid"
#define KXMLQLCVirtualConsoleGridEnabled "Enabled"
#define KXMLQLCVirtualConsoleGridXResolution "XResolution"
#define KXMLQLCVirtualConsoleGridYResolution "YResolution"

#define KXMLQLCVirtualConsoleKeyboard "Keyboard"
#define KXMLQLCVirtualConsoleKeyboardGrab "Grab"
#define KXMLQLCVirtualConsoleKeyboardRepeat "Repeat"

#define KXMLQLCVirtualConsoleFrameStyle "FrameStyle"

#define KXMLQLCVCAppearance "Appearance"
#define KXMLQLCVCForegroundColor "ForegroundColor"
#define KXMLQLCVCBackgroundColor "BackgroundColor"
#define KXMLQLCVCColorDefault "Default"
#define KXMLQLCVCFont "Font"
#define KXMLQLCVCFontDefault "Default"
#define KXMLQLCVCBackgroundImage "BackgroundImage"
#define KXMLQLCVCBackgroundImageNone "None"

const int KFrameStyleSunken ( QFrame::StyledPanel | QFrame::Sunken );
const int KFrameStyleRaised ( QFrame::StyledPanel | QFrame::Raised );
const int KFrameStyleNone ( QFrame::NoFrame );

// Menu stuff
const int KVCMenuEvent            ( 2000 );

// Add Menu >>>
const int KVCMenuAddMin           ( 100 );
const int KVCMenuAddMax           ( 199 );
const int KVCMenuAddButton        ( 101 );
const int KVCMenuAddSlider        ( 102 );
const int KVCMenuAddFrame         ( 103 );
const int KVCMenuAddLabel         ( 104 );
const int KVCMenuAddXYPad         ( 105 );
// <<< Add Menu

// Tools Menu >>>
const int KVCMenuToolsMin         ( 200 );
const int KVCMenuToolsMax         ( 299 );
const int KVCMenuToolsSettings    ( 200 );
const int KVCMenuToolsSliders     ( 201 );
const int KVCMenuToolsPanic       ( 202 );
// <<< Tools Menu

// Edit Menu >>>
const int KVCMenuEditMin          ( 300 );
const int KVCMenuEditMax          ( 399 );
const int KVCMenuEditCut          ( 300 );
const int KVCMenuEditCopy         ( 301 );
const int KVCMenuEditPaste        ( 302 );
const int KVCMenuEditDelete       ( 303 );
const int KVCMenuEditProperties   ( 304 );
const int KVCMenuEditRename       ( 305 );
// <<< Edit Menu

// Foreground Menu >>>
const int KVCMenuForegroundMin    ( 400 );
const int KVCMenuForegroundMax    ( 499 );
const int KVCMenuForegroundFont   ( 400 );
const int KVCMenuForegroundColor  ( 401 );
const int KVCMenuForegroundNone   ( 402 );
// <<< Foreground Menu

// Background Menu >>>
const int KVCMenuBackgroundMin    ( 500 );
const int KVCMenuBackgroundMax    ( 599 );
const int KVCMenuBackgroundFrame  ( 500 );
const int KVCMenuBackgroundColor  ( 501 );
const int KVCMenuBackgroundPixmap ( 502 );
const int KVCMenuBackgroundNone   ( 503 );
// <<< Foreground Menu

// Stacking Menu >>>
const int KVCMenuStackingMin      ( 600 );
const int KVCMenuStackingMax      ( 699 );
const int KVCMenuStackingRaise    ( 600 );
const int KVCMenuStackingLower    ( 601 );
// <<< Stacking Menu

class VCMenuEvent : public QCustomEvent
{
 public:
	VCMenuEvent(int item) : QCustomEvent(KVCMenuEvent), m_menuItem(item) {}
  
	int menuItem() { return m_menuItem; }

protected:
	int m_menuItem;
};


class VirtualConsole : public QWidget
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
 public:
	VirtualConsole(QWidget* parent);
	~VirtualConsole();

	void initView();

 protected:
	void initMenuBar();
	void initDockArea();

	/*********************************************************************
	 * Grid
	 *********************************************************************/
public:
	/** Set widget placement grid enabled/disabled */
	void setGridEnabled(bool enable) { m_gridEnabled = enable; }

	/** Get widget placement grid state */
	bool isGridEnabled() { return m_gridEnabled; }

	/** Set widget placement grid X resolution */
	void setGridX(int x) { m_gridX = x; }

	/** Get widget placement grid X resolution */
	int gridX() { return m_gridX; }

	/** Set widget placement grid Y resolution */
	void setGridY(int y) { m_gridY = y; }

	/** Get widget placement grid Y resolution */
	int gridY() { return m_gridY; }

protected:
	bool m_gridEnabled;
	int m_gridX;
	int m_gridY;

	/*********************************************************************
	 * Keyboard state
	 *********************************************************************/
public:
	void setKeyRepeatOff(bool set) { m_keyRepeatOff = set; }
	bool isKeyRepeatOff() { return m_keyRepeatOff; }

	void setGrabKeyboard(bool grab) { m_grabKeyboard = grab; }
	bool isGrabKeyboard() { return m_grabKeyboard; }

protected:
	bool m_keyRepeatOff;
	bool m_grabKeyboard;

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	/** Load virtual console contents from a file */
	static bool loader(QDomDocument* doc, QDomElement* vc_root);

	/** Load Virtual Console contents from an XML document */
	bool loadXML(QDomDocument* doc, QDomElement* root);

	/** Load Virtual Console contents from an XML document */
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

	/*********************************************************************
	 * Selected widget
	 *********************************************************************/
public:
	/** Get the currently selected widget */
	QWidget* selectedWidget() { return m_selectedWidget; }

	/** Set the currently selected widget */
	void setSelectedWidget(QWidget*);

protected:
	QWidget* m_selectedWidget;

	/*********************************************************************
	 * Draw area
	 *********************************************************************/
 public:
	void setDrawArea(VCFrame* drawArea);
	VCFrame* drawArea() const { return m_drawArea; }

 protected:
	VCFrame* m_drawArea;

	/*********************************************************************
	 * Menus
	 *********************************************************************/
public:
	QPopupMenu* toolsMenu() { return m_toolsMenu; }
	QPopupMenu* editMenu() { return m_editMenu; }
	QPopupMenu* addMenu() { return m_addMenu; }

protected:
	QPopupMenu* m_toolsMenu;
	QPopupMenu* m_addMenu;
	QPopupMenu* m_editMenu;

	/*********************************************************************
	 * Misc slots
	 *********************************************************************/
 public slots:
	void slotDockAreaHidden(bool);
	void slotModeChanged();

	/*********************************************************************
	 * Add menu callbacks
	 *********************************************************************/
	void slotAddButton();
	void slotAddSlider();
	void slotAddFrame();
	void slotAddLabel();
	void slotAddXYPad();

	/*********************************************************************
	 * Tools menu callbacks
	 *********************************************************************/
	void slotToolsSliders();
	void slotToolsSettings();
	void slotToolsPanic();

	/*********************************************************************
	 * Edit menu callbacks
	 *********************************************************************/
	void slotEditCut();
	void slotEditCopy();
	void slotEditPaste();
	void slotEditDelete();
	void slotEditProperties();
	void slotEditRename();
	void slotEditRenameReturnPressed();
	void slotEditRenameCancelled();

	/*********************************************************************
	 * Foreground menu callbacks
	 *********************************************************************/
	void slotForegroundFont();
	void slotForegroundColor();
	void slotForegroundNone();

	/*********************************************************************
	 * Background menu callbacks
	 *********************************************************************/
	void slotBackgroundColor();
	void slotBackgroundImage();
	void slotBackgroundNone();
	void slotBackgroundFrame();

	/*********************************************************************
	 * Stacking menu callbacks
	 *********************************************************************/
	void slotStackingRaise();
	void slotStackingLower();

	/*********************************************************************
	 * Signals
	 *********************************************************************/
signals:
	void closed();
	void InpEvent(const int, const int, const int);
	void sendFeedBack();

	void keyPressed(QKeyEvent*);
	void keyReleased(QKeyEvent*);

	/*********************************************************************
	 * Event handlers
	 *********************************************************************/
protected:
	void closeEvent(QCloseEvent* e);
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);
	void customEvent(QCustomEvent*);

	/*********************************************************************
	 * Frame style converters
	 *********************************************************************/
public:
	static QString frameStyleToString(const int style);
	static int stringToFrameStyle(const QString& style);

	/*********************************************************************
	 * Misc member attributes
	 *********************************************************************/
protected:
	// Master layout
	QHBoxLayout* m_layout; 

	// Virtual console menu bar
	QMenuBar* m_menuBar;

	// Dock area
	VCDockArea* m_dockArea;

	// Key receiver bind objects
	QPtrList <KeyBind> m_keyReceivers;

	// Rename edit
	FloatingEdit* m_renameEdit;
};

#endif
