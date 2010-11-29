/*
  Q Light Controller
  virtualconsole.cpp

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

#include <QDesktopWidget>
#include <QMdiSubWindow>
#include <QApplication>
#include <QInputDialog>
#include <QColorDialog>
#include <QActionGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QFontDialog>
#include <QScrollArea>
#include <QKeyEvent>
#include <QMdiArea>
#include <QMenuBar>
#include <QToolBar>
#include <QString>
#include <QDebug>
#include <QMenu>
#include <QList>
#include <QtXml>

#include "vcpropertieseditor.h"
#include "addvcbuttonmatrix.h"
#include "addvcslidermatrix.h"
#include "virtualconsole.h"
#include "vcproperties.h"
#include "vcdockslider.h"
#include "vcsoloframe.h"
#include "mastertimer.h"
#include "vcdockarea.h"
#include "outputmap.h"
#include "vccuelist.h"
#include "inputmap.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
#include "vclabel.h"
#include "vcxypad.h"
#include "app.h"
#include "doc.h"

#ifndef WIN32
#include <X11/Xlib.h>
#endif

extern App* _app;
extern QApplication* _qapp;

VCProperties VirtualConsole::s_properties;
VirtualConsole* VirtualConsole::s_instance = NULL;

/****************************************************************************
 * Initialization
 ****************************************************************************/

VirtualConsole::VirtualConsole(QWidget* parent, Qt::WindowFlags flags)
        : QWidget(parent, flags)
{
    m_editActionGroup = NULL;
    m_addActionGroup = NULL;
    m_bgActionGroup = NULL;
    m_fgActionGroup = NULL;
    m_fontActionGroup = NULL;
    m_frameActionGroup = NULL;
    m_stackingActionGroup = NULL;

    m_customMenu = NULL;
    m_toolsMenu = NULL;
    m_editMenu = NULL;
    m_addMenu = NULL;

    m_dockArea = NULL;
    m_scrollArea = NULL;

    m_editAction = EditNone;
    m_editMenu = NULL;

    /* Main layout */
    new QHBoxLayout(this);

    initActions();
    initMenuBar();

    initDockArea();
    initContents();

    connect(_app->masterTimer(), SIGNAL(functionListChanged()),
            this, SLOT(slotRunningFunctionsChanged()));
    slotRunningFunctionsChanged();

    connect(_app->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
            this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));

    connect(_app->outputMap(), SIGNAL(blackoutChanged(bool)),
            this, SLOT(slotBlackoutChanged(bool)));

    slotModeChanged(_app->doc()->mode());
}

VirtualConsole::~VirtualConsole()
{
    /* The layout takes ownership of the contents. Adopt them back to the
       main application object to prevent their destruction. */
    if (_app != NULL)
        s_properties.m_contents->setParent(_app);

#ifdef __APPLE__
    s_properties.store(this);
#else
    s_properties.store(parentWidget());
#endif

    s_instance = NULL;
}

void VirtualConsole::create(QWidget* parent)
{
    QWidget* window;

    /* Must not create more than one instance */
    if (s_instance != NULL)
        return;

#ifdef __APPLE__
    /* Create a separate window for OSX */
    s_instance = new VirtualConsole(parent, Qt::Window);
    window = s_instance;
#else
    /* Create an MDI window for X11 & Win32 */
    QMdiArea* area = qobject_cast<QMdiArea*> (_app->centralWidget());
    Q_ASSERT(area != NULL);
    s_instance = new VirtualConsole(parent);
    window = area->addSubWindow(s_instance);
#endif

    /* Listen to mode changes */
    connect(_app->doc(), SIGNAL(modeChanged(Doc::Mode)),
            s_instance, SLOT(slotModeChanged(Doc::Mode)));

    /* Set some common properties for the window and show it */
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowIcon(QIcon(":/virtualconsole.png"));
    window->setWindowTitle(tr("Virtual Console"));
    window->setContextMenuPolicy(Qt::CustomContextMenu);
    window->setWindowState(s_properties.state());
    window->setGeometry(s_properties.x(), s_properties.y(),
                        s_properties.width(), s_properties.height());
    window->show();
}

/*****************************************************************************
 * Selected widget
 *****************************************************************************/

void VirtualConsole::setWidgetSelected(VCWidget* widget, bool select)
{
    Q_ASSERT(widget != NULL);

    if (select == false)
    {
        m_selectedWidgets.removeAll(widget);
        widget->update();
    }
    else if (select == true && m_selectedWidgets.indexOf(widget) == -1)
    {
        m_selectedWidgets.append(widget);
        widget->update();
    }

    /* Change the custom menu to the latest-selected widget's menu */
    updateCustomMenu();

    /* Enable or disable actions */
    updateActions();
}

bool VirtualConsole::isWidgetSelected(VCWidget* widget) const
{
    if (widget == NULL || m_selectedWidgets.indexOf(widget) == -1)
        return false;
    else
        return true;
}

void VirtualConsole::clearWidgetSelection()
{
    /* Get a copy of selected widget list */
    QList <VCWidget*> widgets(m_selectedWidgets);

    /* Clear the list so isWidgetSelected() returns false for all widgets */
    m_selectedWidgets.clear();

    /* Update all widgets to clear the selection frame around them */
    QListIterator <VCWidget*> it(widgets);
    while (it.hasNext() == true)
        it.next()->update();

    /* Change the custom menu to the latest-selected widget's menu */
    updateCustomMenu();

    /* Enable or disable actions */
    updateActions();
}

/*****************************************************************************
 * Actions, menu- and toolbar
 *****************************************************************************/

void VirtualConsole::initActions()
{
    /* Add menu actions */
    m_addButtonAction = new QAction(QIcon(":/button.png"),
                                    tr("Button"), this);
    connect(m_addButtonAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddButton()));

    m_addButtonMatrixAction = new QAction(QIcon(":/buttonmatrix.png"),
                                          tr("Button Matrix"), this);
    connect(m_addButtonMatrixAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddButtonMatrix()));

    m_addSliderAction = new QAction(QIcon(":/slider.png"),
                                    tr("Slider"), this);
    connect(m_addSliderAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSlider()));

    m_addSliderMatrixAction = new QAction(QIcon(":/slidermatrix.png"),
                                          tr("Slider Matrix"), this);
    connect(m_addSliderMatrixAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSliderMatrix()));

    m_addXYPadAction = new QAction(QIcon(":/xypad.png"),
                                   tr("XY pad"), this);
    connect(m_addXYPadAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddXYPad()));

    m_addCueListAction = new QAction(QIcon(":/cuelist.png"),
                                     tr("Cue list"), this);
    connect(m_addCueListAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddCueList()));

    m_addFrameAction = new QAction(QIcon(":/frame.png"),
                                   tr("Frame"), this);
    connect(m_addFrameAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddFrame()));

    m_addSoloFrameAction = new QAction(QIcon(":/soloframe.png"),
                       tr("Solo frame"), this);
    connect(m_addSoloFrameAction, SIGNAL(triggered(bool)),
        this, SLOT(slotAddSoloFrame()));

    m_addLabelAction = new QAction(QIcon(":/label.png"),
                                   tr("Label"), this);
    connect(m_addLabelAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddLabel()));

    /* Put add actions under the same group */
    m_addActionGroup = new QActionGroup(this);
    m_addActionGroup->setExclusive(false);
    m_addActionGroup->addAction(m_addButtonAction);
    m_addActionGroup->addAction(m_addButtonMatrixAction);
    m_addActionGroup->addAction(m_addSliderAction);
    m_addActionGroup->addAction(m_addSliderMatrixAction);
    m_addActionGroup->addAction(m_addXYPadAction);
    m_addActionGroup->addAction(m_addCueListAction);
    m_addActionGroup->addAction(m_addFrameAction);
    m_addActionGroup->addAction(m_addSoloFrameAction);
    m_addActionGroup->addAction(m_addLabelAction);

    /* Tools menu actions */
    m_toolsSettingsAction = new QAction(QIcon(":/configure.png"),
                                        tr("Settings"), this);
    connect(m_toolsSettingsAction, SIGNAL(triggered(bool)),
            this, SLOT(slotToolsSettings()));
    // Prevent this action from ending up to the application menu on OSX
    // and crashing the app after VC window is closed.
    m_toolsSettingsAction->setMenuRole(QAction::NoRole);

    m_toolsSlidersAction = new QAction(QIcon(":/slider.png"),
                                       tr("Default sliders"), this);
    connect(m_toolsSlidersAction, SIGNAL(triggered(bool)),
            this, SLOT(slotToolsSliders()));

    m_toolsBlackoutAction = new QAction(QIcon(":/blackout.png"),
                                        tr("Toggle &Blackout"), this);
    m_toolsBlackoutAction->setCheckable(true);
    connect(m_toolsBlackoutAction, SIGNAL(triggered(bool)),
            this, SLOT(slotToolsBlackout()));
    Q_ASSERT(_app->outputMap());
    m_toolsBlackoutAction->setChecked(_app->outputMap()->blackout());

    m_toolsPanicAction = new QAction(QIcon(":/panic.png"),
                                     tr("Stop ALL functions!"), this);
    m_toolsPanicAction->setShortcut(QKeySequence("CTRL+SHIFT+ESC"));
    connect(m_toolsPanicAction, SIGNAL(triggered(bool)),
            this, SLOT(slotToolsPanic()));

    /* Edit menu actions */
    m_editCutAction = new QAction(QIcon(":/editcut.png"),
                                  tr("Cut"), this);
    connect(m_editCutAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEditCut()));

    m_editCopyAction = new QAction(QIcon(":/editcopy.png"),
                                   tr("Copy"), this);
    connect(m_editCopyAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEditCopy()));

    m_editPasteAction = new QAction(QIcon(":/editpaste.png"),
                                    tr("Paste"), this);
    m_editPasteAction->setEnabled(false);
    connect(m_editPasteAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEditPaste()));

    m_editDeleteAction = new QAction(QIcon(":/editdelete.png"),
                                     tr("Delete"), this);
    connect(m_editDeleteAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEditDelete()));

    m_editPropertiesAction = new QAction(QIcon(":/configure.png"),
                                         tr("Properties"), this);
    connect(m_editPropertiesAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEditProperties()));

    m_editRenameAction = new QAction(QIcon(":/editclear.png"),
                                     tr("Rename"), this);
    connect(m_editRenameAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEditRename()));

    /* Put edit actions under the same group */
    m_editActionGroup = new QActionGroup(this);
    m_editActionGroup->setExclusive(false);
    m_editActionGroup->addAction(m_editCutAction);
    m_editActionGroup->addAction(m_editCopyAction);
    m_editActionGroup->addAction(m_editPasteAction);
    m_editActionGroup->addAction(m_editDeleteAction);
    m_editActionGroup->addAction(m_editPropertiesAction);
    m_editActionGroup->addAction(m_editRenameAction);

    /* Background menu actions */
    m_bgColorAction = new QAction(QIcon(":/color.png"),
                                  tr("Color"), this);
    connect(m_bgColorAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBackgroundColor()));

    m_bgImageAction = new QAction(QIcon(":/image.png"),
                                  tr("Image"), this);
    connect(m_bgImageAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBackgroundImage()));

    m_bgDefaultAction = new QAction(QIcon(":/undo.png"),
                                    tr("Default"), this);
    connect(m_bgDefaultAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBackgroundNone()));

    /* Put BG actions under the same group */
    m_bgActionGroup = new QActionGroup(this);
    m_bgActionGroup->setExclusive(false);
    m_bgActionGroup->addAction(m_bgColorAction);
    m_bgActionGroup->addAction(m_bgImageAction);
    m_bgActionGroup->addAction(m_bgDefaultAction);

    /* Foreground menu actions */
    m_fgColorAction = new QAction(QIcon(":/color.png"),
                                  tr("Color"), this);
    connect(m_fgColorAction, SIGNAL(triggered(bool)),
            this, SLOT(slotForegroundColor()));

    m_fgDefaultAction = new QAction(QIcon(":/undo.png"),
                                    tr("Default"), this);
    connect(m_fgDefaultAction, SIGNAL(triggered(bool)),
            this, SLOT(slotForegroundNone()));

    /* Put FG actions under the same group */
    m_fgActionGroup = new QActionGroup(this);
    m_fgActionGroup->setExclusive(false);
    m_fgActionGroup->addAction(m_fgColorAction);
    m_fgActionGroup->addAction(m_fgDefaultAction);

    /* Font menu actions */
    m_fontAction = new QAction(QIcon(":/fonts.png"),
                               tr("Choose..."), this);
    connect(m_fontAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFont()));

    m_resetFontAction = new QAction(QIcon(":/undo.png"),
                                    tr("Default"), this);
    connect(m_resetFontAction, SIGNAL(triggered(bool)),
            this, SLOT(slotResetFont()));

    /* Put font actions under the same group */
    m_fontActionGroup = new QActionGroup(this);
    m_fontActionGroup->setExclusive(false);
    m_fontActionGroup->addAction(m_fontAction);
    m_fontActionGroup->addAction(m_resetFontAction);

    /* Frame menu actions */
    m_frameSunkenAction = new QAction(QIcon(":/framesunken.png"),
                                      tr("Sunken"), this);
    connect(m_frameSunkenAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFrameSunken()));

    m_frameRaisedAction = new QAction(QIcon(":/frameraised.png"),
                                      tr("Raised"), this);
    connect(m_frameRaisedAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFrameRaised()));

    m_frameNoneAction = new QAction(QIcon(":/framenone.png"),
                                    tr("None"), this);
    connect(m_frameNoneAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFrameNone()));

    /* Put frame actions under the same group */
    m_frameActionGroup = new QActionGroup(this);
    m_frameActionGroup->setExclusive(false);
    m_frameActionGroup->addAction(m_frameRaisedAction);
    m_frameActionGroup->addAction(m_frameSunkenAction);
    m_frameActionGroup->addAction(m_frameNoneAction);

    /* Stacking menu actions */
    m_stackingRaiseAction = new QAction(QIcon(":/up.png"),
                                        tr("Raise"), this);
    connect(m_stackingRaiseAction, SIGNAL(triggered(bool)),
            this, SLOT(slotStackingRaise()));

    m_stackingLowerAction = new QAction(QIcon(":/down.png"),
                                        tr("Lower"), this);
    connect(m_stackingLowerAction, SIGNAL(triggered(bool)),
            this, SLOT(slotStackingLower()));

    /* Put stacking actions under the same group */
    m_stackingActionGroup = new QActionGroup(this);
    m_stackingActionGroup->setExclusive(false);
    m_stackingActionGroup->addAction(m_stackingRaiseAction);
    m_stackingActionGroup->addAction(m_stackingLowerAction);
}

void VirtualConsole::initMenuBar()
{
    QMenuBar* menuBar;
    QToolBar* toolBar;
    QVBoxLayout* vbox;
    QWidget* widget;

    /* Menu widget to the layout */
    widget = new QWidget(this);
    vbox = new QVBoxLayout(widget);
    vbox->setMargin(0);
    layout()->setMenuBar(widget);

    /* Menubar */
#ifndef __APPLE__
    menuBar = new QMenuBar(this);
    vbox->addWidget(menuBar);
#else
    menuBar = new QMenuBar(this);
#endif

    /* Add menu */
    m_addMenu = new QMenu(menuBar);
    m_addMenu->setTitle(tr("&Add"));
    menuBar->addMenu(m_addMenu);
    m_addMenu->addAction(m_addButtonAction);
    m_addMenu->addAction(m_addButtonMatrixAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addSliderAction);
    m_addMenu->addAction(m_addSliderMatrixAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addXYPadAction);
    m_addMenu->addAction(m_addCueListAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addFrameAction);
    m_addMenu->addAction(m_addSoloFrameAction);
    m_addMenu->addAction(m_addLabelAction);

    /* Edit menu */
    m_editMenu = new QMenu(menuBar);
    m_editMenu->setTitle(tr("&Edit"));
    menuBar->addMenu(m_editMenu);
    m_editMenu->addAction(m_editCutAction);
    m_editMenu->addAction(m_editCopyAction);
    m_editMenu->addAction(m_editPasteAction);
    m_editMenu->addAction(m_editDeleteAction);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_editPropertiesAction);
    m_editMenu->addAction(m_editRenameAction);
    m_editMenu->addSeparator();

    /* Tools menu */
    m_toolsMenu = new QMenu(menuBar);
    m_toolsMenu->setTitle(tr("&Tools"));
    menuBar->addMenu(m_toolsMenu);
    m_toolsMenu->addAction(m_toolsBlackoutAction);
    m_toolsMenu->addAction(m_toolsPanicAction);
    m_toolsMenu->addAction(m_toolsSlidersAction);
    m_toolsMenu->addSeparator();
    m_toolsMenu->addAction(m_toolsSettingsAction);

    /* Background Menu */
    QMenu* bgMenu = new QMenu(m_editMenu);
    bgMenu->setTitle(tr("&Background"));
    m_editMenu->addMenu(bgMenu);
    bgMenu->addAction(m_bgColorAction);
    bgMenu->addAction(m_bgImageAction);
    bgMenu->addAction(m_bgDefaultAction);

    /* Foreground menu */
    QMenu* fgMenu = new QMenu(m_editMenu);
    fgMenu->setTitle(tr("&Foreground"));
    m_editMenu->addMenu(fgMenu);
    fgMenu->addAction(m_fgColorAction);
    fgMenu->addAction(m_fgDefaultAction);

    /* Font menu */
    QMenu* fontMenu = new QMenu(m_editMenu);
    fontMenu->setTitle(tr("F&ont"));
    m_editMenu->addMenu(fontMenu);
    fontMenu->addAction(m_fontAction);
    fontMenu->addAction(m_resetFontAction);

    /* Frame menu */
    QMenu* frameMenu = new QMenu(m_editMenu);
    frameMenu->setTitle(tr("F&rame"));
    m_editMenu->addMenu(frameMenu);
    frameMenu->addAction(m_frameSunkenAction);
    frameMenu->addAction(m_frameRaisedAction);
    frameMenu->addAction(m_frameNoneAction);

    /* Stacking order menu */
    QMenu* stackMenu = new QMenu(m_editMenu);
    stackMenu->setTitle(tr("Stacking &order"));
    m_editMenu->addMenu(stackMenu);
    stackMenu->addAction(m_stackingRaiseAction);
    stackMenu->addAction(m_stackingLowerAction);

    /* Add a separator that separates the common edit items from a custom
       widget menu that gets appended to the edit menu when a selected
       widget provides one. */
    m_editMenu->addSeparator();

    /* Toolbar */
    toolBar = new QToolBar(widget);
    vbox->addWidget(toolBar);

    toolBar->addAction(m_addButtonAction);
    toolBar->addAction(m_addButtonMatrixAction);
    toolBar->addAction(m_addSliderAction);
    toolBar->addAction(m_addSliderMatrixAction);
    toolBar->addAction(m_addXYPadAction);
    toolBar->addAction(m_addCueListAction);
    toolBar->addAction(m_addFrameAction);
    toolBar->addAction(m_addSoloFrameAction);
    toolBar->addAction(m_addLabelAction);
    toolBar->addSeparator();
    toolBar->addAction(m_editCutAction);
    toolBar->addAction(m_editCopyAction);
    toolBar->addAction(m_editPasteAction);
    toolBar->addSeparator();
    toolBar->addAction(m_editDeleteAction);
    toolBar->addSeparator();
    toolBar->addAction(m_editPropertiesAction);
    toolBar->addAction(m_editRenameAction);

    QWidget* spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding,
                                QSizePolicy::Preferred);
    toolBar->addWidget(spacerWidget);
    toolBar->addAction(m_toolsBlackoutAction);
    toolBar->addAction(m_toolsPanicAction);
}

void VirtualConsole::updateCustomMenu()
{
    /* Get rid of the custom menu, but delete it later because this might
       be called from the very menu that is being deleted. */
    if (m_customMenu != NULL)
    {
        delete m_customMenu;
        m_customMenu = NULL;
    }

    if (m_selectedWidgets.size() > 0)
    {
        /* Change the custom menu to the last selected widget's menu */
        VCWidget* latestWidget = m_selectedWidgets.last();
        m_customMenu = latestWidget->customMenu(m_editMenu);
        if (m_customMenu != NULL)
            m_editMenu->addMenu(m_customMenu);
    }
    else
    {
        /* Change the custom menu to the bottom frame's menu */
        Q_ASSERT(contents() != NULL);
        m_customMenu = contents()->customMenu(m_editMenu);
        if (m_customMenu != NULL)
            m_editMenu->addMenu(m_customMenu);
    }
}

void VirtualConsole::updateActions()
{
    /* When selected widgets is empty, all actions go to main draw area. */
    if (m_selectedWidgets.isEmpty() == true)
    {
        /* Enable widget additions to draw area */
        m_addActionGroup->setEnabled(true);

        /* Disable edit actions that can't be allowed for draw area */
        m_editCutAction->setEnabled(false);
        m_editCopyAction->setEnabled(false);
        m_editDeleteAction->setEnabled(false);
        m_editRenameAction->setEnabled(false);
        m_editPropertiesAction->setEnabled(false);

        /* All the rest are disabled for draw area, except BG & font */
        m_fgActionGroup->setEnabled(false);
        m_frameActionGroup->setEnabled(false);
        m_stackingActionGroup->setEnabled(false);

        /* Enable paste to draw area if there's something to paste */
        if (m_clipboard.isEmpty() == true)
            m_editPasteAction->setEnabled(false);
        else
            m_editPasteAction->setEnabled(true);
    }
    else
    {
        /* Enable edit actions for other widgets */
        m_editCutAction->setEnabled(true);
        m_editCopyAction->setEnabled(true);
        m_editDeleteAction->setEnabled(true);
        m_editRenameAction->setEnabled(true);
        m_editPropertiesAction->setEnabled(true);

        /* Enable all common properties */
        m_bgActionGroup->setEnabled(true);
        m_fgActionGroup->setEnabled(true);
        m_fontActionGroup->setEnabled(true);
        m_frameActionGroup->setEnabled(true);
        m_stackingActionGroup->setEnabled(true);

        /* Check, whether the last selected widget can hold children */
        if (m_selectedWidgets.last()->canHaveChildren() == true)
        {
            /* Enable paste for widgets that can hold children */
            if (m_clipboard.isEmpty() == true)
                m_editPasteAction->setEnabled(false);
            else
                m_editPasteAction->setEnabled(true);

            /* Enable also new additions */
            m_addActionGroup->setEnabled(true);
        }
        else
        {
            /* No pasted children possible */
            m_editPasteAction->setEnabled(false);
        }
    }
}

void VirtualConsole::slotRunningFunctionsChanged()
{
    if (_app->masterTimer() && _app->masterTimer()->runningFunctions() > 0)
        m_toolsPanicAction->setEnabled(true);
    else
        m_toolsPanicAction->setEnabled(false);
}

/*****************************************************************************
 * Add menu callbacks
 *****************************************************************************/

VCFrame* VirtualConsole::closestParent() const
{
    /* Either add to the draw area or the latest selected widget or one of
       its parents */
    if (m_selectedWidgets.isEmpty() == true)
    {
        return contents();
    }
    else
    {
        /* Traverse upwards in parent hierarchy and find the next
           VCFrame */
        VCFrame* parent = NULL;
        VCWidget* widget = m_selectedWidgets.last();
        while (parent == NULL && widget != NULL)
        {
            // Attempt to cast the VCWidget into VCFrame
            parent = qobject_cast<VCFrame*> (widget);
            if (parent != NULL)
            {
                // Found a VCFrame parent widget
                break;
            }
            else
            {
                // Cast failed, so it's not a VCFrame.
                // Try the widget's parent widget. If it has
                // none or it's not a VCWidget, the loop
                // should end on the next pass & parent == NULL
                widget = qobject_cast<VCWidget*>
                         (widget->parentWidget());
            }
        }

        return parent;
    }
}

void VirtualConsole::slotAddButton()
{
    VCFrame* parent(closestParent());
    if (parent == NULL)
        return;

    VCButton* button = new VCButton(parent);
    Q_ASSERT(button != NULL);
    button->show();
    button->move(parent->lastClickPoint());
    _app->doc()->setModified();
}

void VirtualConsole::slotAddButtonMatrix()
{
    VCFrame* parent(closestParent());
    if (parent == NULL)
        return;

    AddVCButtonMatrix abm(this);
    if (abm.exec() == QDialog::Rejected)
        return;

    int h = abm.horizontalCount();
    int v = abm.verticalCount();
    int sz = abm.buttonSize();

    VCWidget* frame = NULL;
    if (abm.frameStyle() == AddVCButtonMatrix::NormalFrame)
        frame = new VCFrame(parent);
    else
        frame = new VCSoloFrame(parent);
    Q_ASSERT(frame != NULL);

    // Resize the parent frame to fit the buttons nicely
    frame->resize(QSize((h * sz) + 20, (v * sz) + 20));

    for (int y = 0; y < v; y++)
    {
        for (int x = 0; x < h; x++)
        {
            VCButton* button = new VCButton(frame);
            Q_ASSERT(button != NULL);
            button->move(QPoint(10 + (x * sz), 10 + (y * sz)));
            button->resize(QSize(sz, sz));
            button->show();

            int index = (y * h) + x;
            if (index < abm.functions().size())
            {
                t_function_id fid = abm.functions().at(index);
                Function* function = _app->doc()->function(fid);
                if (function != NULL)
                {
                    button->setFunction(fid);
                    button->setCaption(function->name());
                }
            }
        }
    }

    // Show the frame after adding buttons to prevent flickering
    frame->show();
    frame->move(parent->lastClickPoint());
    _app->doc()->setModified();
}

void VirtualConsole::slotAddSlider()
{
    VCFrame* parent(closestParent());
    if (parent == NULL)
        return;

    VCSlider* slider = new VCSlider(parent);
    Q_ASSERT(slider != NULL);
    slider->show();
    slider->move(parent->lastClickPoint());
    _app->doc()->setModified();
}

void VirtualConsole::slotAddSliderMatrix()
{
    VCFrame* parent(closestParent());
    if (parent == NULL)
        return;

    AddVCSliderMatrix avsm(this);
    if (avsm.exec() == QDialog::Rejected)
        return;

    int width = VCSlider::defaultSize.width();
    int height = avsm.height();
    int count = avsm.amount();

    VCFrame* frame = new VCFrame(parent);
    Q_ASSERT(frame != NULL);

    // Resize the parent frame to fit the sliders nicely
    frame->resize(QSize((count * width) + 20, height + 20));

    for (int i = 0; i < count; i++)
    {
        VCSlider* slider = new VCSlider(frame);
        Q_ASSERT(slider != NULL);
        slider->move(QPoint(10 + (width * i), 10));
        slider->resize(QSize(width, height));
        slider->show();
    }

    // Show the frame after adding buttons to prevent flickering
    frame->show();
    frame->move(parent->lastClickPoint());
    _app->doc()->setModified();
}

void VirtualConsole::slotAddXYPad()
{
    VCFrame* parent(closestParent());
    if (parent == NULL)
        return;

    VCXYPad* xypad = new VCXYPad(parent);
    Q_ASSERT(xypad != NULL);
    xypad->show();
    xypad->move(parent->lastClickPoint());
    _app->doc()->setModified();
}

void VirtualConsole::slotAddCueList()
{
    VCFrame* parent(closestParent());
    if (parent == NULL)
        return;

    VCCueList* cuelist = new VCCueList(parent);
    Q_ASSERT(cuelist != NULL);
    cuelist->show();
    cuelist->move(parent->lastClickPoint());
    _app->doc()->setModified();
}

void VirtualConsole::slotAddFrame()
{
    VCFrame* parent(closestParent());
    if (parent == NULL)
        return;

    VCFrame* frame = new VCFrame(parent);
    Q_ASSERT(frame != NULL);
    frame->show();
    frame->move(parent->lastClickPoint());
    _app->doc()->setModified();
}

void VirtualConsole::slotAddSoloFrame()
{
    VCFrame* parent(closestParent());
    if (parent == NULL)
        return;

    VCSoloFrame* soloframe = new VCSoloFrame(parent);
    Q_ASSERT(soloframe != NULL);
    soloframe->show();
    soloframe->move(parent->lastClickPoint());
    _app->doc()->setModified();
}

void VirtualConsole::slotAddLabel()
{
    VCFrame* parent(closestParent());
    if (parent == NULL)
        return;

    VCLabel* label = new VCLabel(parent);
    Q_ASSERT(label != NULL);
    label->show();
    label->move(parent->lastClickPoint());
    _app->doc()->setModified();
}

/*****************************************************************************
 * Tools menu callbacks
 *****************************************************************************/

void VirtualConsole::slotToolsSettings()
{
    VCPropertiesEditor vcpe(this, s_properties);
    if (vcpe.exec() == QDialog::Accepted)
    {
        s_properties = vcpe.properties();
        m_dockArea->refreshProperties();
        _app->doc()->setModified();
    }
}

void VirtualConsole::slotToolsSliders()
{
    if (m_dockArea->isHidden() == true)
    {
        s_properties.setSlidersVisible(true);
        m_dockArea->show();
    }
    else
    {
        s_properties.setSlidersVisible(false);
        m_dockArea->hide();
    }

    _app->doc()->setModified();
}

void VirtualConsole::slotToolsBlackout()
{
    Q_ASSERT(_app->outputMap() != NULL);
    _app->outputMap()->setBlackout(!_app->outputMap()->blackout());
    if (s_properties.blackoutInputUniverse() != InputMap::invalidUniverse() &&
        s_properties.blackoutInputChannel() != KInputChannelInvalid)
    {
        uchar value = (_app->outputMap()->blackout()) ? 255 : 0;
        _app->inputMap()->feedBack(s_properties.blackoutInputUniverse(),
                                   s_properties.blackoutInputChannel(),
                                   value);
    }
}

void VirtualConsole::slotToolsPanic()
{
    Q_ASSERT(_app->masterTimer() != NULL);
    _app->masterTimer()->stopAllFunctions();
}

void VirtualConsole::slotBlackoutChanged(bool state)
{
    m_toolsBlackoutAction->setChecked(state);
}

/*****************************************************************************
 * Edit menu callbacks
 *****************************************************************************/

void VirtualConsole::slotEditCut()
{
    /* No need to delete widgets in clipboard because they are actually just
       MOVED to another parent during Paste when m_editAction == EditCut.
       Cutting the widgets does nothing to them unless Paste is invoked. */

    /* Make the edit action valid only if there's something to cut */
    if (m_selectedWidgets.size() == 0)
    {
        m_editAction = EditNone;
        m_clipboard.clear();
        m_editPasteAction->setEnabled(false);
    }
    else
    {
        m_editAction = EditCut;
        m_clipboard = m_selectedWidgets;
        m_editPasteAction->setEnabled(true);
    }
}

void VirtualConsole::slotEditCopy()
{
    /* Make the edit action valid only if there's something to copy */
    if (m_selectedWidgets.size() == 0)
    {
        m_editAction = EditNone;
        m_clipboard.clear();
        m_editPasteAction->setEnabled(false);
    }
    else
    {
        m_editAction = EditCopy;
        m_clipboard = m_selectedWidgets;
        m_editPasteAction->setEnabled(true);
    }
}

void VirtualConsole::slotEditPaste()
{
    if (m_clipboard.size() == 0)
    {
        /* Invalidate the edit action if there's nothing to paste */
        m_editAction = EditNone;
        m_editPasteAction->setEnabled(false);
        return;
    }

    VCWidget* parent;
    VCWidget* widget;
    QRect bounds;

    Q_ASSERT(contents() != NULL);

    /* Select the parent that gets the cut clipboard contents */
    if (m_selectedWidgets.size() == 0)
        parent = contents();
    else
        parent = m_selectedWidgets.last();

    /* Get the bounding rect for all selected widgets */
    QListIterator <VCWidget*> it(m_clipboard);
    while (it.hasNext() == true)
    {
        widget = it.next();
        Q_ASSERT(widget != NULL);
        bounds = bounds.united(widget->geometry());
    }

    /* Get the upcoming parent's last mouse click point */
    QPoint cp(parent->lastClickPoint());

    if (m_editAction == EditCut)
    {
        it.toFront();
        while (it.hasNext() == true)
        {
            widget = it.next();
            Q_ASSERT(widget != NULL);

            /* Get widget's relative pos to the bounding rect */
            QPoint p(widget->x() - bounds.x() + cp.x(),
                     widget->y() - bounds.y() + cp.y());

            /* Reparent and move to the correct place */
            widget->setParent(parent);
            widget->move(p);
            widget->show();
        }

        /* Clear clipboard after pasting stuff that was CUT */
        m_clipboard.clear();
        m_editPasteAction->setEnabled(false);
    }
    else if (m_editAction == EditCopy)
    {
        it.toFront();
        while (it.hasNext() == true)
        {
            widget = it.next();
            Q_ASSERT(widget != NULL);

            /* Get widget's relative pos to the bounding rect */
            QPoint p(widget->x() - bounds.x() + cp.x(),
                     widget->y() - bounds.y() + cp.y());

            /* Create a copy and move to correct place */
            VCWidget* copy = widget->createCopy(parent);
            Q_ASSERT(copy != NULL);
            copy->move(p);
            copy->show();
        }
    }
}

void VirtualConsole::slotEditDelete()
{
    QString msg(tr("Do you wish to delete the selected widgets?"));
    QString title(tr("Delete widgets"));
    int result = QMessageBox::question(this, title, msg,
                                       QMessageBox::Yes,
                                       QMessageBox::No);
    if (result == QMessageBox::Yes)
    {
        while (m_selectedWidgets.isEmpty() == false)
        {
            /* Consume the selected list until it is empty and
               delete each widget. */
            VCWidget* widget = m_selectedWidgets.takeFirst();
            widget->deleteLater();

            /* Remove the widget from clipboard as well so that
               deleted widgets won't be pasted anymore anywhere */
            m_clipboard.removeAll(widget);
            m_editPasteAction->setEnabled(false);
        }
    }
}

void VirtualConsole::slotEditProperties()
{
    VCWidget* widget;

    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        widget = contents();
    else
        widget = m_selectedWidgets.last();

    if (widget != NULL)
        widget->editProperties();
}

void VirtualConsole::slotEditRename()
{
    if (m_selectedWidgets.isEmpty() == true)
        return;

    bool ok = false;
    QString text(m_selectedWidgets.last()->caption());
    text = QInputDialog::getText(this, tr("Rename widgets"), tr("Caption:"),
                                 QLineEdit::Normal, text, &ok);
    if (ok == true)
    {
        VCWidget* widget;
        foreach(widget, m_selectedWidgets)
            widget->setCaption(text);
    }
}

/*****************************************************************************
 * Background menu callbacks
 *****************************************************************************/

void VirtualConsole::slotBackgroundColor()
{
    QColor color;

    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        color = contents()->backgroundColor();
    else
        color = m_selectedWidgets.last()->backgroundColor();

    color = QColorDialog::getColor(color);
    if (color.isValid() == true)
    {
        if (m_selectedWidgets.isEmpty() == true)
        {
            contents()->setBackgroundColor(color);
        }
        else
        {
            VCWidget* widget;
            foreach(widget, m_selectedWidgets)
                widget->setBackgroundColor(color);
        }
    }
}

void VirtualConsole::slotBackgroundImage()
{
    QString path;

    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        path = contents()->backgroundImage();
    else
        path = m_selectedWidgets.last()->backgroundImage();

    path = QFileDialog::getOpenFileName(this,
                                        tr("Select background image"),
                                        path,
                                        "Images (*.png *.xpm *.jpg *.gif)");
    if (path.isEmpty() == false)
    {
        if (m_selectedWidgets.isEmpty() == true)
        {
            contents()->setBackgroundImage(path);
        }
        else
        {
            VCWidget* widget;
            foreach(widget, m_selectedWidgets)
                widget->setBackgroundImage(path);
        }
    }
}

void VirtualConsole::slotBackgroundNone()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
    {
        contents()->resetBackgroundColor();
    }
    else
    {
        VCWidget* widget;
        foreach(widget, m_selectedWidgets)
            widget->resetBackgroundColor();
    }
}

/*****************************************************************************
 * Foreground menu callbacks
 *****************************************************************************/

void VirtualConsole::slotForegroundColor()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    QColor color(m_selectedWidgets.last()->foregroundColor());
    color = QColorDialog::getColor(color);
    if (color.isValid() == true)
    {
        VCWidget* widget;
        foreach(widget, m_selectedWidgets)
            widget->setForegroundColor(color);
    }
}

void VirtualConsole::slotForegroundNone()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    VCWidget* widget;
    foreach(widget, m_selectedWidgets)
        widget->resetForegroundColor();
}

/*****************************************************************************
 * Font menu callbacks
 *****************************************************************************/

void VirtualConsole::slotFont()
{
    bool ok = false;
    QFont font;

    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        font = contents()->font();
    else
        font = m_selectedWidgets.last()->font();

    /* This crashes with Qt 4.6.x on OSX. Upgrade to 4.7.x. */
    font = QFontDialog::getFont(&ok, font);
    if (ok == true)
    {
        if (m_selectedWidgets.isEmpty() == true)
        {
            contents()->setFont(font);
        }
        else
        {
            VCWidget* widget;
            foreach(widget, m_selectedWidgets)
                widget->setFont(font);
        }
    }
}

void VirtualConsole::slotResetFont()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
    {
        contents()->resetFont();
    }
    else
    {
        VCWidget* widget;
        foreach(widget, m_selectedWidgets)
            widget->resetFont();
    }
}

/*****************************************************************************
 * Stacking menu callbacks
 *****************************************************************************/

void VirtualConsole::slotStackingRaise()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    VCWidget* widget;
    foreach(widget, m_selectedWidgets)
        widget->raise();
}

void VirtualConsole::slotStackingLower()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    VCWidget* widget;
    foreach(widget, m_selectedWidgets)
        widget->lower();
}

/*****************************************************************************
 * Frame menu callbacks
 *****************************************************************************/

void VirtualConsole::slotFrameSunken()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    VCWidget* widget;
    foreach(widget, m_selectedWidgets)
        widget->setFrameStyle(KVCFrameStyleSunken);
}

void VirtualConsole::slotFrameRaised()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    VCWidget* widget;
    foreach(widget, m_selectedWidgets)
        widget->setFrameStyle(KVCFrameStyleRaised);
}

void VirtualConsole::slotFrameNone()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    VCWidget* widget;
    foreach(widget, m_selectedWidgets)
        widget->setFrameStyle(KVCFrameStyleNone);
}

/*****************************************************************************
 * Dock area
 *****************************************************************************/

void VirtualConsole::initDockArea()
{
    if (m_dockArea != NULL)
        delete m_dockArea;

    m_dockArea = new VCDockArea(this);
    m_dockArea->setSizePolicy(QSizePolicy::Maximum,
                              QSizePolicy::Expanding);

    // Add the dock area into the master horizontal layout
    layout()->addWidget(m_dockArea);

    /* Show the dock area by default */
    m_dockArea->show();
}

/*****************************************************************************
 * Contents
 *****************************************************************************/

void VirtualConsole::resetContents()
{
    /* Destroy existing contents */
    if (s_properties.m_contents != NULL)
    {
        delete s_properties.m_contents;
        s_properties.m_contents = NULL;
    }

    /* If there is an instance of the VC, make it re-read the contents */
    if (s_instance != NULL)
    {
        s_instance->dockArea()->refreshProperties();
        s_instance->initContents();
    }
}

void VirtualConsole::initContents()
{
    Q_ASSERT(layout() != NULL);

    /* Create new contents if there isn't any yet */
    if (contents() == NULL)
        s_properties.resetContents();

    /* Add the contents area into the master horizontal layout */
    if (m_scrollArea == NULL)
    {
        m_scrollArea = new QScrollArea(this);
        layout()->addWidget(m_scrollArea);
        m_scrollArea->setAlignment(Qt::AlignCenter);
        m_scrollArea->setWidgetResizable(false);
    }

    /* Make the bottom frame as big as the screen */
    QDesktopWidget dw;
    contents()->setGeometry(dw.availableGeometry(this));
    contents()->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
                                          QSizePolicy::Fixed));
    m_scrollArea->setWidget(contents());

    /* Disconnect old key handlers to prevent duplicates */
    disconnect(this, SIGNAL(keyPressed(const QKeySequence&)),
               contents(), SLOT(slotKeyPressed(const QKeySequence&)));
    disconnect(this, SIGNAL(keyReleased(const QKeySequence&)),
               contents(), SLOT(slotKeyReleased(const QKeySequence&)));

    /* Connect new key handlers */
    connect(this, SIGNAL(keyPressed(const QKeySequence&)),
            contents(), SLOT(slotKeyPressed(const QKeySequence&)));
    connect(this, SIGNAL(keyReleased(const QKeySequence&)),
            contents(), SLOT(slotKeyReleased(const QKeySequence&)));

    /* Make the contents area take up all available space */
    contents()->setSizePolicy(QSizePolicy::Expanding,
                              QSizePolicy::Expanding);

    m_clipboard.clear();
    m_selectedWidgets.clear();

    /* Update actions' enabled status */
    updateActions();
}

/*****************************************************************************
 * Key press handler
 *****************************************************************************/

void VirtualConsole::keyPressEvent(QKeyEvent* event)
{
    QKeySequence seq(event->key() | event->modifiers());
    emit keyPressed(seq);
}

void VirtualConsole::keyReleaseEvent(QKeyEvent* event)
{
    QKeySequence seq(event->key() | event->modifiers());
    emit keyReleased(seq);
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VirtualConsole::slotInputValueChanged(quint32 uni, quint32 ch, uchar value)
{
    if (uni == s_properties.blackoutInputUniverse() &&
        ch == s_properties.blackoutInputChannel() && value > 0)
    {
        slotToolsBlackout();
    }
}

/*****************************************************************************
 * Main application mode
 *****************************************************************************/

void VirtualConsole::slotModeChanged(Doc::Mode mode)
{
    QString config;

    /* Key repeat */
    if (s_properties.isKeyRepeatOff() == true)
    {
#if !defined(WIN32) && !defined(__APPLE__)
        Display* display;
        display = XOpenDisplay(NULL);
        Q_ASSERT(display != NULL);

        if (mode == Doc::Design)
            XAutoRepeatOn(display);
        else
            XAutoRepeatOff(display);

        XCloseDisplay(display);
#else
#endif
    }

    /* Grab keyboard */
    if (s_properties.isGrabKeyboard() == true)
    {
        if (mode == Doc::Design)
            releaseKeyboard();
        else
            grabKeyboard();
    }

    if (mode == Doc::Operate)
    {
        // Don't allow editing or adding in operate mode
        m_toolsSettingsAction->setEnabled(false);
        m_editActionGroup->setEnabled(false);
        m_addActionGroup->setEnabled(false);
        m_bgActionGroup->setEnabled(false);
        m_fgActionGroup->setEnabled(false);
        m_fontActionGroup->setEnabled(false);
        m_frameActionGroup->setEnabled(false);
        m_stackingActionGroup->setEnabled(false);

        // Disable action shortcuts in operate mode
        m_addButtonAction->setShortcut(QKeySequence());
        m_addButtonMatrixAction->setShortcut(QKeySequence());
        m_addSliderAction->setShortcut(QKeySequence());
        m_addSliderMatrixAction->setShortcut(QKeySequence());
        m_addXYPadAction->setShortcut(QKeySequence());
        m_addCueListAction->setShortcut(QKeySequence());
        m_addFrameAction->setShortcut(QKeySequence());
        m_addSoloFrameAction->setShortcut(QKeySequence());
        m_addLabelAction->setShortcut(QKeySequence());

        m_editCutAction->setShortcut(QKeySequence());
        m_editCopyAction->setShortcut(QKeySequence());
        m_editPasteAction->setShortcut(QKeySequence());
        m_editDeleteAction->setShortcut(QKeySequence());
        m_editPropertiesAction->setShortcut(QKeySequence());

        m_bgColorAction->setShortcut(QKeySequence());
        m_bgImageAction->setShortcut(QKeySequence());
        m_bgDefaultAction->setShortcut(QKeySequence());
        m_fgColorAction->setShortcut(QKeySequence());
        m_fgDefaultAction->setShortcut(QKeySequence());
        m_fontAction->setShortcut(QKeySequence());
        m_resetFontAction->setShortcut(QKeySequence());
        m_frameSunkenAction->setShortcut(QKeySequence());
        m_frameRaisedAction->setShortcut(QKeySequence());
        m_frameNoneAction->setShortcut(QKeySequence());

        m_stackingRaiseAction->setShortcut(QKeySequence());
        m_stackingLowerAction->setShortcut(QKeySequence());
    }
    else
    {
        // Allow editing and adding in design mode
        m_toolsSettingsAction->setEnabled(true);
        m_editActionGroup->setEnabled(true);
        m_addActionGroup->setEnabled(true);
        m_bgActionGroup->setEnabled(true);
        m_fgActionGroup->setEnabled(true);
        m_fontActionGroup->setEnabled(true);
        m_frameActionGroup->setEnabled(true);
        m_stackingActionGroup->setEnabled(true);

        // Set action shortcuts for design mode
        m_addButtonAction->setShortcut(QKeySequence("CTRL+SHIFT+B"));
        m_addButtonMatrixAction->setShortcut(QKeySequence("CTRL+SHIFT+M"));
        m_addSliderAction->setShortcut(QKeySequence("CTRL+SHIFT+S"));
        m_addSliderMatrixAction->setShortcut(QKeySequence("CTRL+SHIFT+I"));
        m_addXYPadAction->setShortcut(QKeySequence("CTRL+SHIFT+X"));
        m_addCueListAction->setShortcut(QKeySequence("CTRL+SHIFT+C"));
        m_addFrameAction->setShortcut(QKeySequence("CTRL+SHIFT+F"));
        m_addSoloFrameAction->setShortcut(QKeySequence("CTRL+SHIFT+O"));
        m_addLabelAction->setShortcut(QKeySequence("CTRL+SHIFT+L"));

        m_editCutAction->setShortcut(QKeySequence("CTRL+X"));
        m_editCopyAction->setShortcut(QKeySequence("CTRL+C"));
        m_editPasteAction->setShortcut(QKeySequence("CTRL+V"));
        m_editDeleteAction->setShortcut(QKeySequence("Delete"));
        m_editPropertiesAction->setShortcut(QKeySequence("CTRL+E"));

        m_bgColorAction->setShortcut(QKeySequence("SHIFT+B"));
        m_bgImageAction->setShortcut(QKeySequence("SHIFT+I"));
        m_bgDefaultAction->setShortcut(QKeySequence("SHIFT+ALT+B"));
        m_fgColorAction->setShortcut(QKeySequence("SHIFT+F"));
        m_fgDefaultAction->setShortcut(QKeySequence("SHIFT+ALT+F"));
        m_fontAction->setShortcut(QKeySequence("SHIFT+O"));
        m_resetFontAction->setShortcut(QKeySequence("SHIFT+ALT+O"));
        m_frameSunkenAction->setShortcut(QKeySequence("SHIFT+S"));
        m_frameRaisedAction->setShortcut(QKeySequence("SHIFT+R"));
        m_frameNoneAction->setShortcut(QKeySequence("SHIFT+ALT+S"));

        m_stackingRaiseAction->setShortcut(QKeySequence("SHIFT+UP"));
        m_stackingLowerAction->setShortcut(QKeySequence("SHIFT+DOWN"));
    }
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VirtualConsole::loadXML(const QDomElement* vc_root)
{
    bool retval = false;

    Q_ASSERT(vc_root != NULL);

    /* Load properties & contents */
    retval = s_properties.loadXML(vc_root);

    /* Display VC if appropriate */
    if (s_properties.visible() == true)
        create(_app);

    /* Make the dock area update itself after loading its settings. The
       contents area is already updated. */
    if (s_instance != NULL)
        s_instance->dockArea()->refreshProperties();

    return retval;
}

bool VirtualConsole::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Store instance properties (geometry) */
    if (s_instance != NULL)
    {
#ifdef __APPLE__
        s_properties.store(s_instance);
#else
        s_properties.store(s_instance->parentWidget());
#endif
    }

    return s_properties.saveXML(doc, wksp_root);
}
