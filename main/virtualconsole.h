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

#include <QWidget>
#include <QFrame>
#include <QList>

#include "app.h"

class QDomDocument;
class QActionGroup;
class QDomElement;
class QAction;
class QMenu;

class VCDockArea;
class VCWidget;
class VCFrame;
class KeyBind;

#define KXMLQLCVirtualConsole "VirtualConsole"

#define KXMLQLCVirtualConsoleGrid "Grid"
#define KXMLQLCVirtualConsoleGridEnabled "Enabled"
#define KXMLQLCVirtualConsoleGridXResolution "XResolution"
#define KXMLQLCVirtualConsoleGridYResolution "YResolution"

#define KXMLQLCVirtualConsoleKeyboard "Keyboard"
#define KXMLQLCVirtualConsoleKeyboardGrab "Grab"
#define KXMLQLCVirtualConsoleKeyboardRepeat "Repeat"

#define KXMLQLCVirtualConsoleTimerType "Timer"
#define KXMLQLCVirtualConsoleTimerSoftware "Software"
#define KXMLQLCVirtualConsoleTimerHardware "Hardware"

#define KXMLQLCVCAppearance "Appearance"
#define KXMLQLCVCFrameStyle "FrameStyle"
#define KXMLQLCVCForegroundColor "ForegroundColor"
#define KXMLQLCVCBackgroundColor "BackgroundColor"
#define KXMLQLCVCColorDefault "Default"
#define KXMLQLCVCFont "Font"
#define KXMLQLCVCFontDefault "Default"
#define KXMLQLCVCBackgroundImage "BackgroundImage"
#define KXMLQLCVCBackgroundImageNone "None"

/** Frame styles */
#define KFrameStyleSunken QFrame::StyledPanel | QFrame::Sunken
#define KFrameStyleRaised QFrame::StyledPanel | QFrame::Raised
#define KFrameStyleNone   QFrame::NoFrame

class VirtualConsole : public QWidget
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	VirtualConsole(QWidget* parent);
	~VirtualConsole();

private:
	Q_DISABLE_COPY(VirtualConsole)

protected:
	void initActions();
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
	 * Selected widgets
	 *********************************************************************/
public:
	enum EditAction { EditNone, EditCut, EditCopy };

	/** Set the edit action for selected widgets */
	void setEditAction(EditAction action) { m_editAction = action; }

	/** Get the edit action for selected widgets */
	EditAction editAction() const { return m_editAction; }

	/** Get a list of currently selected widgets */
	const QList <VCWidget*> selectedWidgets() const
		{ return m_selectedWidgets; }

	/** Either select or unselect a widget */
	void setWidgetSelected(VCWidget* widget, bool selected);

	/** Check, whether the given widget is selected */
	bool isWidgetSelected(VCWidget* widget) const;

	/** Clear the list of selected widgets */
	void clearWidgetSelection();

protected:
	/** Change the custom menu to the last selected widget's menu */
	void updateCustomMenu();

	/** Enable or disable actions based on current selection */
	void updateActions();

protected:
	/** The widgets that are currently selected */
	QList <VCWidget*> m_selectedWidgets;

	/** The widgets that are currently either copied or cut */
	QList <VCWidget*> m_clipboard;

	/** Indicates, whether the selected widgets should be copied or cut */
	EditAction m_editAction;

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
	 * Menus & actions
	 *********************************************************************/
public:
	QMenu* customMenu() { return m_customMenu; }
	QMenu* toolsMenu() { return m_toolsMenu; }
	QMenu* editMenu() { return m_editMenu; }
	QMenu* addMenu() { return m_addMenu; }

protected:
	QActionGroup* m_addActionGroup;
	QActionGroup* m_editActionGroup;
	QActionGroup* m_bgActionGroup;
	QActionGroup* m_fgActionGroup;
	QActionGroup* m_fontActionGroup;
	QActionGroup* m_frameActionGroup;
	QActionGroup* m_stackingActionGroup;
	
	QAction* m_addButtonAction;
	QAction* m_addSliderAction;
	QAction* m_addXYPadAction;
	QAction* m_addCueListAction;
	QAction* m_addFrameAction;
	QAction* m_addLabelAction;

	QAction* m_toolsSettingsAction;
	QAction* m_toolsSlidersAction;
	QAction* m_toolsPanicAction;

	QAction* m_editCutAction;
	QAction* m_editCopyAction;
	QAction* m_editPasteAction;
	QAction* m_editDeleteAction;
	QAction* m_editPropertiesAction;
	QAction* m_editRenameAction;

	QAction* m_bgColorAction;
	QAction* m_bgImageAction;
	QAction* m_bgDefaultAction;

	QAction* m_fgColorAction;
	QAction* m_fgDefaultAction;

	QAction* m_fontAction;
	QAction* m_resetFontAction;

	QAction* m_frameSunkenAction;
	QAction* m_frameRaisedAction;
	QAction* m_frameNoneAction;

	QAction* m_stackingRaiseAction;
	QAction* m_stackingLowerAction;

protected:
	QMenu* m_customMenu;
	QMenu* m_toolsMenu;
	QMenu* m_editMenu;
	QMenu* m_addMenu;
	
	/*********************************************************************
	 * Misc slots
	 *********************************************************************/
public slots:
	void slotModeChanged(App::Mode mode);

signals:
	void modeChanged(App::Mode);

	/*********************************************************************
	 * Add menu callbacks
	 *********************************************************************/
public slots:
	void slotAddButton();
	void slotAddSlider();
	void slotAddXYPad();
	void slotAddCueList();
	void slotAddFrame();
	void slotAddLabel();

	/*********************************************************************
	 * Tools menu callbacks
	 *********************************************************************/
public slots:
	void slotToolsSliders();
	void slotToolsSettings();
	void slotToolsPanic();

	/*********************************************************************
	 * Edit menu callbacks
	 *********************************************************************/
public slots:
	void slotEditCut();
	void slotEditCopy();
	void slotEditPaste();
	void slotEditDelete();
	void slotEditRename();
	void slotEditProperties();

	/*********************************************************************
	 * Background menu callbacks
	 *********************************************************************/
public slots:
	void slotBackgroundColor();
	void slotBackgroundImage();
	void slotBackgroundNone();

	/*********************************************************************
	 * Foreground menu callbacks
	 *********************************************************************/
public slots:
	void slotForegroundColor();
	void slotForegroundNone();

	/*********************************************************************
	 * Font menu callbacks
	 *********************************************************************/
public slots:
	void slotFont();
	void slotResetFont();

	/*********************************************************************
	 * Frame menu callbacks
	 *********************************************************************/
public slots:
	void slotFrameSunken();
	void slotFrameRaised();
	void slotFrameNone();

	/*********************************************************************
	 * Stacking menu callbacks
	 *********************************************************************/
public slots:
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
	 * Key event receivers
	 *********************************************************************/
protected:
	// Key receiver bind objects
	QList <KeyBind*> m_keyReceivers;
};

#endif
