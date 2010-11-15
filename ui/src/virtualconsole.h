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

#include <QKeySequence>
#include <QWidget>
#include <QFrame>
#include <QList>

#include "vcproperties.h"
#include "app.h"

class VirtualConsole;
class QDomDocument;
class QActionGroup;
class QScrollArea;
class QDomElement;
class VCDockArea;
class QKeyEvent;
class VCWidget;
class VCFrame;
class QAction;
class KeyBind;
class QMenu;

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
    Q_DISABLE_COPY(VirtualConsole)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** Get the VC singleton instance. Can be NULL. */
    static VirtualConsole* instance() {
        return s_instance;
    }

    /** Create a VC with parent. Fails if s_instance is not NULL. */
    static void create(QWidget* parent);

    /** Public destructor */
    ~VirtualConsole();

protected:
    /** Protected constructor to prevent multiple instances */
    VirtualConsole(QWidget* parent, Qt::WindowFlags flags = 0);

protected:
    /** The singleton instance */
    static VirtualConsole* s_instance;

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /** Get VC properties read-only */
    static const VCProperties properties() {
        return s_properties;
    }

protected:
    /** VC properties */
    static VCProperties s_properties;

    /*********************************************************************
     * Selected widgets
     *********************************************************************/
public:
    enum EditAction { EditNone, EditCut, EditCopy };

    /** Set the edit action for selected widgets */
    void setEditAction(EditAction action) {
        m_editAction = action;
    }

    /** Get the edit action for selected widgets */
    EditAction editAction() const {
        return m_editAction;
    }

    /** Get a list of currently selected widgets */
    const QList <VCWidget*> selectedWidgets() const
    {
        return m_selectedWidgets;
    }

    /** Either select or unselect a widget */
    void setWidgetSelected(VCWidget* widget, bool selected);

    /** Check, whether the given widget is selected */
    bool isWidgetSelected(VCWidget* widget) const;

    /** Clear the list of selected widgets */
    void clearWidgetSelection();

protected:
    /** The widgets that are currently selected */
    QList <VCWidget*> m_selectedWidgets;

    /** The widgets that are currently either copied or cut */
    QList <VCWidget*> m_clipboard;

    /** Indicates, whether the selected widgets should be copied or cut */
    EditAction m_editAction;

    /*********************************************************************
     * Actions, menu- and toolbar
     *********************************************************************/
public:
    QMenu* customMenu() {
        return m_customMenu;
    }
    QMenu* toolsMenu() {
        return m_toolsMenu;
    }
    QMenu* editMenu() {
        return m_editMenu;
    }
    QMenu* addMenu() {
        return m_addMenu;
    }

protected:
    /** Initialize actions */
    void initActions();

    /** Initialize menus and toolbar */
    void initMenuBar();

    /** Change the custom menu to the last selected widget's menu */
    void updateCustomMenu();

    /** Enable or disable actions based on current selection */
    void updateActions();

protected slots:
    void slotRunningFunctionsChanged();

protected:
    QToolBar* m_toolbar;

    QActionGroup* m_addActionGroup;
    QActionGroup* m_editActionGroup;
    QActionGroup* m_bgActionGroup;
    QActionGroup* m_fgActionGroup;
    QActionGroup* m_fontActionGroup;
    QActionGroup* m_frameActionGroup;
    QActionGroup* m_stackingActionGroup;

    QAction* m_addButtonAction;
    QAction* m_addButtonMatrixAction;
    QAction* m_addSliderAction;
    QAction* m_addSliderMatrixAction;
    QAction* m_addXYPadAction;
    QAction* m_addCueListAction;
    QAction* m_addFrameAction;
    QAction* m_addSoloFrameAction;
    QAction* m_addLabelAction;

    QAction* m_toolsSettingsAction;
    QAction* m_toolsSlidersAction;
    QAction* m_toolsBlackoutAction;
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
     * Add menu callbacks
     *********************************************************************/
private:
    /**
     * Attempt to find the closest parent for new widget. It's either
     * the bottom frame or a newly-added VCFrame or the parent of another
     * newly-added (non-VCFrame) widget. If a parent cannot be found for
     * some reason, NULL is returned.
     *
     * @return Closest parent VCFrame*
     */
    VCFrame* closestParent() const;

public slots:
    void slotAddButton();
    void slotAddButtonMatrix();
    void slotAddSlider();
    void slotAddSliderMatrix();
    void slotAddXYPad();
    void slotAddCueList();
    void slotAddFrame();
    void slotAddSoloFrame();
    void slotAddLabel();

    /*********************************************************************
     * Tools menu callbacks
     *********************************************************************/
public slots:
    void slotToolsSliders();
    void slotToolsSettings();
    void slotToolsBlackout();
    void slotToolsPanic();

    void slotBlackoutChanged(bool state);

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
     * Dock Area
     *********************************************************************/
public:
    /** Get a pointer to the dock area that holds the default sliders */
    VCDockArea* dockArea() {
        return m_dockArea;
    }

protected:
    /** Initialize default sliders */
    void initDockArea();

protected:
    /** Dock area that holds the default fade & hold sliders */
    VCDockArea* m_dockArea;

    /*********************************************************************
     * Contents
     *********************************************************************/
public:
    /** Get the VC's current contents */
    VCFrame* contents() const {
        return s_properties.contents();
    }

    /** Reset the VC contents to an initial state */
    static void resetContents();

protected:
    /** Place the contents area to the VC view */
    void initContents();

protected:
    /** Scroll Area that contains the bottom-most VCFrame */
    QScrollArea* m_scrollArea;

    /*********************************************************************
     * Key press handler
     *********************************************************************/
protected:
    /** Handler for keyboard key presse events */
    void keyPressEvent(QKeyEvent* event);

    /** Handler for keyboard key release events */
    void keyReleaseEvent(QKeyEvent* event);

signals:
    /** Signal telling that the keySequence was pressed down */
    void keyPressed(const QKeySequence& keySequence);

    /** Signal telling that the keySequence was released */
    void keyReleased(const QKeySequence& keySequence);

    /*************************************************************************
     * External input
     *************************************************************************/
public slots:
    /** Listens to external input data */
    void slotInputValueChanged(quint32 uni, quint32 ch, uchar value);

    /*********************************************************************
     * Main application mode
     *********************************************************************/
public slots:
    /** Slot that catches main application mode changes */
    void slotModeChanged(Doc::Mode mode);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** Load properties and contents from an XML document */
    static bool loadXML(const QDomElement* vc_root);

    /** Save properties and contents to an XML document */
    static bool saveXML(QDomDocument* doc, QDomElement* wksp_root);
};

#endif
