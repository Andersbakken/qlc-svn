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

#include "app.h"

class QMenuBar;
class QPopupMenu;
class QToolBar;
class QHBoxLayout;
class QFile;
class QFrame;
class QDomDocument;
class QDomElement;

class VCWidget;
class VCFrame;
class KeyBind;
class Bus;
class VCDockArea;

#define KXMLQLCVirtualConsole "VirtualConsole"

#define KXMLQLCVCCaption "Caption"

#define KXMLQLCVirtualConsoleGrid "Grid"
#define KXMLQLCVirtualConsoleGridEnabled "Enabled"
#define KXMLQLCVirtualConsoleGridXResolution "XResolution"
#define KXMLQLCVirtualConsoleGridYResolution "YResolution"

#define KXMLQLCVirtualConsoleKeyboard "Keyboard"
#define KXMLQLCVirtualConsoleKeyboardGrab "Grab"
#define KXMLQLCVirtualConsoleKeyboardRepeat "Repeat"

#define KXMLQLCVCAppearance "Appearance"
#define KXMLQLCVCFrameStyle "FrameStyle"
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
const int KVCMenuAddButton        ( 101 );
const int KVCMenuAddSlider        ( 102 );
const int KVCMenuAddFrame         ( 103 );
const int KVCMenuAddLabel         ( 104 );
const int KVCMenuAddXYPad         ( 105 );
const int KVCMenuAddMax           ( 199 );
// <<< Add Menu

// Tools Menu >>>
const int KVCMenuToolsMin         ( 200 );
const int KVCMenuToolsSettings    ( 200 );
const int KVCMenuToolsSliders     ( 201 );
const int KVCMenuToolsPanic       ( 202 );
const int KVCMenuToolsMax         ( 299 );
// <<< Tools Menu

// Edit Menu >>>
const int KVCMenuEditMin          ( 300 );
const int KVCMenuEditCut          ( 300 );
const int KVCMenuEditCopy         ( 301 );
const int KVCMenuEditPaste        ( 302 );
const int KVCMenuEditDelete       ( 303 );
const int KVCMenuEditProperties   ( 304 );
const int KVCMenuEditRename       ( 305 );
const int KVCMenuEditAttach       ( 306 );
const int KVCMenuEditMax          ( 399 );
// <<< Edit Menu

// Foreground Menu >>>
const int KVCMenuForegroundMin     ( 400 );
const int KVCMenuForegroundFont    ( 400 );
const int KVCMenuForegroundColor   ( 401 );
const int KVCMenuForegroundNone    ( 402 );
const int KVCMenuForegroundDefault ( 499 );
const int KVCMenuForegroundMax     ( 499 );
// <<< Foreground Menu

// Background Menu >>>
const int KVCMenuBackgroundMin     ( 500 );
const int KVCMenuBackgroundFrame   ( 500 );
const int KVCMenuBackgroundColor   ( 501 );
const int KVCMenuBackgroundPixmap  ( 502 );
const int KVCMenuBackgroundImage   ( 502 );
const int KVCMenuBackgroundNone    ( 503 );
const int KVCMenuBackgroundDefault ( 599 );
const int KVCMenuBackgroundMax     ( 599 );
// <<< Foreground Menu

// Font Menu >>>
const int KVCMenuFontMin          ( 600 );
const int KVCMenuFont             ( 600 );
const int KVCMenuFontDefault      ( 699 );
const int KVCMenuFontMax          ( 699 );
// <<< Font Menu

// Frame Menu >>>
const int KVCMenuFrameMin         ( 700 );
const int KVCMenuFrameSunken      ( 700 );
const int KVCMenuFrameRaised      ( 701 );
const int KVCMenuFrameNone        ( 799 );
const int KVCMenuFrameMax         ( 799 );
// <<< Frame Menu

// Stacking Menu >>>
const int KVCMenuStackingMin      ( 800 );
const int KVCMenuStackingRaise    ( 800 );
const int KVCMenuStackingLower    ( 801 );
const int KVCMenuStackingMax      ( 899 );
// <<< Stacking Menu

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
	/** Widget placement grid enabled? */
	bool m_gridEnabled;

	/** Widget placement grid X resolution */
	int m_gridX;

	/** Widget placement grid Y resolution */
	int m_gridY;

	/*********************************************************************
	 * Keyboard state
	 *********************************************************************/
public:
	/** Turn key repeat off or not when in operate mode */
	void setKeyRepeatOff(bool set) { m_keyRepeatOff = set; }

	/** Get the status of turning key repeat off */
	bool isKeyRepeatOff() { return m_keyRepeatOff; }

	/** Grab all keyboard input when in operate mode */
	void setGrabKeyboard(bool grab) { m_grabKeyboard = grab; }

	/** Get the status of grabbing keyboard input */
	bool isGrabKeyboard() { return m_grabKeyboard; }

protected:
	/** Key repeat off status */
	bool m_keyRepeatOff;

	/** Keyboard grabbing status */
	bool m_grabKeyboard;

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	/**
	 * Load and initialize virtual console from an XML document
	 *
	 * @param doc An XML document to load from
	 * @param vc_root A VirtualConsole root node to start loading from
	 */
	static bool loader(QDomDocument* doc, QDomElement* vc_root);

	/**
	 * Load Virtual Console contents from an XML document (already
	 * initialized with VirtualConsole::loader())
	 *
	 * @param doc An XML document to load from
	 * @param root A VirtualConsole root node to start loading from
	 */
	bool loadXML(QDomDocument* doc, QDomElement* root);

	/**
	 * Save Virtual Console contents to an XML document
	 *
	 * @param doc An XML Document to save to
	 * @param wksp_root A WorkSpace root node to save under
	 */
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

	/*********************************************************************
	 * Selected widget
	 *********************************************************************/
public:
	/**
	 * Get the currently selected widget. The selected widget is the one
	 * that has been clicked by the user and is being edited.
	 *
	 * @return The selected widget or NULL if none is selected
	 */
	VCWidget* selectedWidget() { return m_selectedWidget; }

	/**
	 * Set the currently selected widget. This is called by individual
	 * widgets when they are clicked. They always set themselves as the
	 * selected widget.
	 *
	 * @param widget The widget that was last clicked
	 */
	void setSelectedWidget(VCWidget* widget);

protected:
	/** The widget that currently has the "edit focus" in virtual console */
	VCWidget* m_selectedWidget;

	/*********************************************************************
	 * Clipboard
	 *********************************************************************/
public:
	/**
	 * Cut the given widgets from their parents to get pasted to another
	 * parent widget. This does not remove anything from anywhere, it just
	 * marks these widgets "cut". Only when paste is invoked, are they
	 * moved.
	 *
	 * @param widgets A list of widget pointers to cut
	 */
	void cut(QPtrList<QWidget> *widgets);

	/**
	 * Copy the given widgets from their parents to get pasted to another
	 * parent widget. 
	 *
	 * @param widgets A list of widget pointers to copy
	 */
	void copy(QPtrList<QWidget> *widgets);

	/**
	 * Paste clipboard contents to the given parent widget. Items can be
	 * pasted only to VCFrames since other VC widget types do not really
	 * support children.
	 *
	 * @param parent The parent VCFrame to paste the widgets to
	 * @param point Use this point as the top-left corner of the bounding
	 *              box where the widgets are pasted to
	 */
	void paste(VCFrame* parent, QPoint point);

protected:
	/**
	 * Copy a VCWidget into another parent VCFrame
	 *
	 * @param widget The widget to make a copy of
	 * @param parent The parent frame for the copy
	 * @param point The point in the parent to paste to
	 */
	void copyWidget(QWidget* widget, VCFrame* parent, QPoint point);

	/**
	 * Clear the contents of the widget clipboard. This should be done at
	 * least when a widget is deleted to prevent the case where we try to
	 * cut/copy/paste a deleted widget. Another alternative would be to
	 * modify the clipboard contents, but... Delete could also be treated
	 * as a clipboard operation; the selected item is cut from its place,
	 * put into the clipboard and from there on to the trashcan.
	 */
	void clearClipboard();

protected:
	typedef enum _ClipboardAction
	{
		ClipboardNone = 0,
		ClipboardCut,
		ClipboardCopy
	} ClipboardAction;

	ClipboardAction m_clipboardAction;
	QPtrList<QWidget> m_clipboard;

	/*********************************************************************
	 * Draw area
	 *********************************************************************/
public:
	/**
	 * Set the bottom-most VCFrame that acts as the "drawing area" housing
	 * all other vc widgets.
	 *
	 * @param drawArea The VCFrame to set as the bottom-most widget
	 */
	void setDrawArea(VCFrame* drawArea);

	/** Get the virtual console's bottom-most widget */
	VCFrame* drawArea() const { return m_drawArea; }

protected:
	/** The bottom-most widget in virtual console */
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
	void slotDockAreaVisibilityChanged(bool isVisible);
	void slotModeChanged(App::Mode mode);

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
	void slotEditRename();
	void slotEditProperties();

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

	void keyPressed(QKeyEvent*);
	void keyReleased(QKeyEvent*);

	/*********************************************************************
	 * Dock Area
	 *********************************************************************/
public:
	/** Get a pointer to the dock area that holds the default sliders */
	VCDockArea* dockArea() { return m_dockArea; }

protected:
	/** Dock area that holds the default fade & hold sliders */
	VCDockArea* m_dockArea;

	/*********************************************************************
	 * Event handlers
	 *********************************************************************/
protected:
	void closeEvent(QCloseEvent* e);
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);

	/*********************************************************************
	 * Misc member attributes
	 *********************************************************************/
protected:
	// Master layout
	QHBoxLayout* m_layout; 

	// Virtual console menu bar
	QMenuBar* m_menuBar;

	// Key receiver bind objects
	QPtrList <KeyBind> m_keyReceivers;
};

#endif
