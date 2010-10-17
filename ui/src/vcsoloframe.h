/*
  Q Light Controller
  vcsoloframe.h

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

#ifndef VCSOLOFRAME_H
#define VCSOLOFRAME_H

#include "vcwidget.h"
#include "vcframe.h"
#include "function.h"
#include "qlctypes.h"

class QDomDocument;
class QDomElement;
class QMouseEvent;
class QString;

#define KXMLQLCVCSoloFrame "SoloFrame"

class VCSoloFrame : public VCFrame
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCSoloFrame(QWidget* parent);
    virtual ~VCSoloFrame();

private:
    /** Prevent copying thru operator= or copy constructor since QObject's
        parental properties get confused when copied. */
    Q_DISABLE_COPY(VCSoloFrame)

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    /** Create a copy of this widget into the given parent */
    VCWidget* createCopy(VCWidget* parent);

    /*****************************************************************************
    * Solo behaviour
    *****************************************************************************/
protected:
    bool thisIsNearestSoloFrameParent(QWidget* widget);
    
protected slots:
    virtual void slotModeChanged(Doc::Mode mode);
    void slotButtonFunctionStarting();

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    static bool loader(const QDomElement* root, QWidget* parent);
    
protected:    
    virtual QString xmlTagName() const { return KXMLQLCVCSoloFrame; }

    /*********************************************************************
     * Event handlers
     *********************************************************************/
protected:
    virtual void paintEvent(QPaintEvent* e);

};

#endif
