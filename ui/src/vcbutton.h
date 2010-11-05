/*
  Q Light Controller
  vcbutton.h

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

#ifndef VCBUTTON_H
#define VCBUTTON_H

#include <QKeySequence>
#include <QWidget>

#include "qlctypes.h"
#include "vcwidget.h"

class QDomDocument;
class QDomElement;
class QMouseEvent;
class QPaintEvent;
class VCButton;
class QAction;
class QPoint;
class QEvent;

#define KXMLQLCVCButton "Button"
#define KXMLQLCVCButtonIcon "Icon"

#define KXMLQLCVCButtonFunction "Function"
#define KXMLQLCVCButtonFunctionID "ID"

#define KXMLQLCVCButtonAction "Action"
#define KXMLQLCVCButtonActionFlash "Flash"
#define KXMLQLCVCButtonActionToggle "Toggle"

#define KXMLQLCVCButtonKey "Key"

class VCButton : public VCWidget
{
    Q_OBJECT

public:
    /** Minimum size for a button */
    static const QSize minimumSize;

    /** Default size for newly-created buttons */
    static const QSize defaultSize;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCButton(QWidget* parent);
    ~VCButton();

private:
    Q_DISABLE_COPY(VCButton)

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    /** Create a copy of this widget to the given parent */
    VCWidget* createCopy(VCWidget* parent);

protected:
    /** Copy the contents for this widget from another widget */
    bool copyFrom(VCWidget* widget);

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /** Edit this widget's properties */
    void editProperties();

    /*********************************************************************
     * Background image
     *********************************************************************/
public:
    /* Don't allow background image setting for buttons */
    void setBackgroundImage(const QString& path) {
        Q_UNUSED(path);
    }

    /*********************************************************************
     * Background color
     *********************************************************************/
public:
    /** Set the button's background color */
    void setBackgroundColor(const QColor& color);

    /** Get the button's background color */
    QColor backgroundColor() const
    {
        return palette().color(QPalette::Button);
    }

    /** Reset the button's background color to whatever the platform uses */
    void resetBackgroundColor();

    /*********************************************************************
     * Foreground color
     *********************************************************************/
public:
    /** Set the button's foreground color */
    void setForegroundColor(const QColor& color);

    /** Get the button's foreground color */
    QColor foregroundColor() const {
        return palette().color(QPalette::ButtonText);
    }

    /** Reset the button's foreground color to whatever the platform uses */
    virtual void resetForegroundColor();

    /*********************************************************************
     * Button icon
     *********************************************************************/
public:
    QString icon() const {
        return m_icon;
    }
    void setIcon(const QString& icon);

public slots:
    void slotChooseIcon();
    void slotResetIcon();

protected:
    QString m_icon;
    QAction* m_chooseIconAction;
    QAction* m_resetIconAction;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /**
     * Create and load a new VCButton from an XML document
     *
     * @param btn_root A VCButton XML root node
     * @param parent A parent VCFrame for the new VCButton
     * @return true if successful; otherwise false
     */
    static bool loader(const QDomElement* btn_root, QWidget* parent);

    /**
     * Load a VCButton's properties from an XML document node
     *
     * @param doc An XML document to load from
     * @param btn_root A VCButton XML root node containing button properties
     * @return true if successful; otherwise false
     */
    bool loadXML(const QDomElement* btn_root);

    /**
     * Save a VCButton's properties to an XML document node
     *
     * @param doc The master XML document to save to
     * @param frame_root The button's VCFrame XML parent node to save to
     */
    bool saveXML(QDomDocument* doc, QDomElement* frame_root);

protected:
    /** Load a legacy KeyBind node */
    bool loadKeyBind(const QDomElement* key_root);

    /*********************************************************************
     * Button state
     *********************************************************************/
public:
    void setOn(bool on);
    bool isOn() const {
        return m_on;
    }

protected:
    bool isChildOfSoloFrame();
    bool m_on;

    /*********************************************************************
     * Key sequence handler
     *********************************************************************/
public:
    void setKeySequence(const QKeySequence& keySequence);
    QKeySequence keySequence() const {
        return m_keySequence;
    }

protected slots:
    void slotKeyPressed(const QKeySequence& keySequence);
    void slotKeyReleased(const QKeySequence& keySequence);

protected:
    QKeySequence m_keySequence;

    /*********************************************************************
     * External input
     *********************************************************************/
protected slots:
    void slotInputValueChanged(quint32 universe,
                               quint32 channel,
                               uchar value);

    /*********************************************************************
     * Function attachment
     *********************************************************************/
public:
    /**
     * Attach a function to a VCButton. This function is started when the
     * button is pressed down.
     *
     * @param function An ID of a function to attach
     */
    void setFunction(t_function_id function);

    /**
     * Get the ID of the function attached to a VCButton
     *
     * @return The ID of the attached function or Function::invalidId()
     *         if there isn't one
     */
    t_function_id function() const {
        return m_function;
    }

protected slots:
    /** Invalidates the button's function if the function is destroyed */
    void slotFunctionRemoved(t_function_id fid);

protected:
    /** The function that this button is controlling */
    t_function_id m_function;

    /*********************************************************************
     * Button action
     *********************************************************************/
public:
    /**
     * Toggle: Start/stop the assigned function.
     * Flash: Keep the function running as long as the button is kept down.
     */
    enum Action { Toggle, Flash };

    /** Set this button's action */
    void setAction(Action action);

    /** Get this button's action */
    Action action() const {
        return m_action;
    }

    static QString actionToString(Action action);
    static Action stringToAction(const QString& str);

protected:
    Action m_action;

    /*********************************************************************
     * Button press / release handlers
     *********************************************************************/
protected:
    /** Handler for button presses (mouse/key)button down, not click */
    void pressFunction();

    /** Handler for button releases (mouse/key)button up, not click */
    void releaseFunction();

protected slots:
    /** Handler for function running signal */
    void slotFunctionRunning(t_function_id fid);

    /** Handler for function stop signal */
    void slotFunctionStopped(t_function_id fid);

    /** Basically the same as slotFunctionStopped() but for flash signal */
    void slotFunctionFlashing(t_function_id fid, bool state);

    /** Slot for brief widget blink when controlled function stops */
    void slotBlinkReady();

signals:
    /** Signal telling the buttons function was started as a result of a toggle action */
    void functionStarting ();

    /*********************************************************************
    * Custom menu
    *********************************************************************/
public:
    /** Get a custom menu specific to this widget. Must be deleted. */
    QMenu* customMenu(QMenu* parentMenu);

    /*********************************************************************
     * Event Handlers
     *********************************************************************/
protected:
    void paintEvent(QPaintEvent* e);

    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
};

#endif
