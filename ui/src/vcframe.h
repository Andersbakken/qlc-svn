/*
  Q Light Controller
  vcframe.h

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

#ifndef VCFRAME_H
#define VCFRAME_H

#include "vcwidget.h"
#include "qlctypes.h"

class QDomDocument;
class QDomElement;
class QMouseEvent;
class QString;

#define KXMLQLCVCFrame "Frame"
#define KXMLQLCVCFrameButtonBehaviour "ButtonBehaviour"

class VCFrame : public VCWidget
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCFrame(QWidget* parent);
    virtual ~VCFrame();

    void init(bool bottomFrame = false);

    /* Check if this is the virtual console's draw area */
    bool isBottomFrame();

private:
    /** Prevent copying thru operator= or copy constructor since QObject's
        parental properties get confused when copied. */
    Q_DISABLE_COPY(VCFrame)

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    /** Create a copy of this widget into the given parent */
    VCWidget* createCopy(VCWidget* parent);

protected:
    /** Copy the contents for this widget from another widget */
    bool copyFrom(VCWidget* widget);

    /*********************************************************************
     * Capability to have children
     *********************************************************************/
public:
    /** VCFrame can hold children */
    bool canHaveChildren() const {
        return true;
    }

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    static bool loader(const QDomElement* root, QWidget* parent);
    bool loadXML(const QDomElement* vc_root);
    bool saveXML(QDomDocument* doc, QDomElement* vc_root);
	
protected:
	/** Can be overridden by subclasses */
	virtual QString xmlTagName() const { return KXMLQLCVCFrame; }

    /*********************************************************************
     * Custom menu
     *********************************************************************/
public:
    /** Get a custom menu specific to this widget. Ownership is transferred
        to the caller, which must delete the returned menu pointer. */
    virtual QMenu* customMenu(QMenu* parentMenu);

    /*********************************************************************
     * Event handlers
     *********************************************************************/
protected:
    void handleWidgetSelection(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
};

#endif
