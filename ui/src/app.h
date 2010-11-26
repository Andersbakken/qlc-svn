/*
  Q Light Controller
  app.h

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

#ifndef APP_H
#define APP_H

#include <QMainWindow>
#include <QString>
#include <QList>
#include <QFile>

#include "qlcfixturedefcache.h"
#include "qlcoutplugin.h"
#include "qlcinplugin.h"
#include "doc.h"

class QProgressDialog;
class QDomDocument;
class QDomElement;
class QMessageBox;
class QToolButton;
class QFileDialog;
class QStatusBar;
class QMenuBar;
class QToolBar;
class QPixmap;
class QAction;
class QLabel;
class QTimer;
class QMenu;

class DummyOutPlugin;
class DummyInPlugin;
class QLCFixtureDef;
class QLCInPlugin;
class MasterTimer;
class QLCPlugin;
class OutputMap;
class InputMap;
class App;

#define KXMLQLCWorkspace "Workspace"

class App : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(App)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    App();
    ~App();

    static QString longName();
    static QString version();

protected:
    void init();
    void closeEvent(QCloseEvent*);

    /*********************************************************************
     * Progress dialog
     *********************************************************************/
public:
    void createProgressDialog();
    void destroyProgressDialog();

public slots:
    void slotSetProgressText(const QString& text);

protected:
    QProgressDialog* m_progressDialog;

    /*********************************************************************
     * Output mapping
     *********************************************************************/
public:
    OutputMap* outputMap() const {
        return m_outputMap;
    }

protected:
    void initOutputMap();

protected slots:
    void slotOutputMapBlackoutChanged(bool state);
    void slotFlashBlackoutIndicator();

protected:
    OutputMap* m_outputMap;

    /*********************************************************************
     * Input mapping
     *********************************************************************/
public:
    InputMap* inputMap() {
        return m_inputMap;
    }

protected:
    void initInputMap();

protected:
    InputMap* m_inputMap;

    /*********************************************************************
     * Master timer
     *********************************************************************/
public:
    /** Get a pointer to the master timer */
    MasterTimer* masterTimer() {
        return m_masterTimer;
    }

protected:
    /** The function runner object */
    MasterTimer* m_masterTimer;

    /*********************************************************************
     * Doc
     *********************************************************************/
public:
    Doc* doc() {
        return m_doc;
    }
    void newDocument();

signals:
    void documentChanged(Doc* doc);

protected slots:
    void slotDocModified(bool state);

protected:
    void initDoc();

protected:
    Doc* m_doc;

    /*********************************************************************
     * Fixture definitions
     *********************************************************************/
public:
    const QLCFixtureDefCache& fixtureDefCache() const
    {
        return m_fixtureDefCache;
    }

protected:
    /** Load all fixture definitions */
    void loadFixtureDefinitions();

protected:
    /** Available fixture definitions */
    QLCFixtureDefCache m_fixtureDefCache;

    /*********************************************************************
     * Main operating mode
     *********************************************************************/
public slots:
    void slotModeOperate();
    void slotModeDesign();
    void slotModeToggle();

    void slotModeChanged(Doc::Mode mode);

    /*********************************************************************
     * Sane style
     *********************************************************************/
public:
    /** Attempt to get a sane style that replaces windows' crappy sliders
     *  and buttons that don't obey background color setting. */
    static QStyle* saneStyle();

protected:
    static QStyle* s_saneStyle;

    /*********************************************************************
     * Status bar
     *********************************************************************/
protected:
    void initStatusBar();

protected:
    /** Flashing blackout indicator on the status bar */
    QLabel* m_blackoutIndicator;

    /** Periodic timer object for the flashing indicator */
    QTimer* m_blackoutIndicatorTimer;

    /** Mode indicator on the status bar */
    QLabel* m_modeIndicator;

    /** Indicator showing available fixture space */
    QLabel* m_fixtureAllocationIndicator;

    /** Indicator showing available function space */
    QLabel* m_functionAllocationIndicator;

    /*********************************************************************
     * Menus & toolbars
     *********************************************************************/
protected:
    void initActions();
    void initMenuBar();
    void initToolBar();
    QMenuBar *menuBar();

    /** Displays an error message if a file error has occurred */
    bool handleFileError(QFile::FileError error);

public slots:
    bool slotFileNew();
    QFile::FileError slotFileOpen();
    QFile::FileError slotFileSave();
    QFile::FileError slotFileSaveAs();
    void slotFileQuit();

    void slotFixtureManager();
    void slotFunctionManager();
    void slotBusManager();
    void slotOutputManager();
    void slotInputManager();

    void slotControlVC();
    void slotControlMonitor();
#ifndef __APPLE__
    void slotControlFullScreen();
#endif

#ifdef __APPLE__
    void slotWindowMenuAboutToShow();
    void slotWindowMenuItemSelected();
    void slotWindowMinimize();
    void slotWindowAllToFront();
#endif

    void slotHelpIndex();
    void slotHelpAbout();
    void slotHelpAboutQt();

    void slotCustomContextMenuRequested(const QPoint&);

protected:
    QAction* m_fileNewAction;
    QAction* m_fileOpenAction;
    QAction* m_fileSaveAction;
    QAction* m_fileSaveAsAction;
    QAction* m_fileQuitAction;

    QAction* m_fixtureManagerAction;
    QAction* m_functionManagerAction;
    QAction* m_busManagerAction;
    QAction* m_inputManagerAction;
    QAction* m_outputManagerAction;

    QAction* m_modeToggleAction;
    QAction* m_controlVCAction;
    QAction* m_controlMonitorAction;
#ifndef __APPLE__
    QAction* m_controlFullScreenAction;
#endif

#ifdef __APPLE__
    QAction* m_windowMinimizeAction;
    QAction* m_windowAllToFrontAction;
#endif

    QAction* m_helpIndexAction;
    QAction* m_helpAboutAction;
    QAction* m_helpAboutQtAction;

protected:
    QMenu* m_fileMenu;
    QMenu* m_managerMenu;
    QMenu* m_controlMenu;
    QMenu* m_helpMenu;

#ifdef __APPLE__
    QMenu* m_windowMenu;
#endif

    QToolBar* m_toolbar;

    /*********************************************************************
     * Workspace background
     *********************************************************************/
public:
    /** Set workspace background image from the given path */
    void setBackgroundImage(QString path);

    /** Get the current workspace background image path */
    QString backgroundImage() const {
        return m_backgroundImage;
    }

public slots:
    /** Open a file dialog to browse an image for workspace background */
    void slotSetBackgroundImage();

    /** Clear the current workspace background */
    void slotClearBackgroundImage();

protected:
    QString m_backgroundImage;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /**
     * Set the name of the current workspace file
     */
    void setFileName(const QString& fileName);

    /**
     * Get the name of the current workspace file
     */
    QString fileName() const;

    /**
     * Load workspace contents from a file with the given name.
     *
     * @param fileName The name of the file to load from.
     * @return QFile::NoError if successful.
     */
    QFile::FileError loadXML(const QString& fileName);

    /**
     * Load workspace contents from the given XML document.
     *
     * @param doc The XML document to load from.
     */
    bool loadXML(const QDomDocument* doc);

    /**
     * Save workspace contents to a file with the given name. Changes the
     * current workspace file name to the given fileName.
     *
     * @param fileName The name of the file to save to.
     * @return QFile::NoError if successful.
     */
    QFile::FileError saveXML(const QString& fileName);

protected:
    QString m_fileName;
};

#endif
