/*
  Q Light Controller
  vccuelist.h

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

#ifndef VCCUELIST_H
#define VCCUELIST_H

#include <QKeySequence>
#include <QWidget>
#include "vcwidget.h"

class QTreeWidgetItem;
class QDomDocument;
class QDomElement;
class QTreeWidget;

class VCCueListProperties;
class Function;

#define KXMLQLCVCCueList "CueList"
#define KXMLQLCVCCueListFunction "Function"
#define KXMLQLCVCCueListKey "Key"
#define KXMLQLCVCCueListNext "Next"
#define KXMLQLCVCCueListPrevious "Previous"

#define KVCCueListColumnNumber 0
#define KVCCueListColumnName   1
#define KVCCueListColumnID     2

/**
 * VCCueList provides a \ref VirtualConsole widget to control cue lists.
 *
 * @see		VCWidget
 * @see		VirtualConsole
 */
class VCCueList : public VCWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCCueList)

    friend class VCCueListProperties;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** Normal constructor */
    VCCueList(QWidget* parent);

    /** Destructor */
    ~VCCueList();

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
     * Cue list
     *********************************************************************/
public:
    /** Clear the tree widget's list of cues */
    void clear();

    /** Append the given function to the widget's list of cues */
    void append(t_function_id fid);

protected slots:
    /** Removes destroyed functions from the list */
    void slotFunctionRemoved(t_function_id fid);

    /** Updates name in the list if function got changed */
    void slotFunctionChanged(t_function_id fid);

    /** Skip to the next cue */
    void slotNextCue();

    /** Skip to the previous cue */
    void slotPreviousCue();

    /** Slot to catch function stopped signals */
    void slotFunctionStopped(t_function_id fid);

    /** Slot that is called whenever the current item changes (either by
        pressing the key binding or clicking an item with mouse) */
    void slotItemActivated(QTreeWidgetItem* item);

protected:
    QTreeWidget* m_list;
    Function* m_current;

    /*********************************************************************
     * Key sequences
     *********************************************************************/
public:
    /** Keyboard key combination for skipping to the next cue */
    void setNextKeySequence(const QKeySequence& keySequence);
    QKeySequence nextKeySequence() const {
        return m_nextKeySequence;
    }

    /** Keyboard key combination for skipping to the previous cue */
    void setPreviousKeySequence(const QKeySequence& keySequence);
    QKeySequence previousKeySequence() const {
        return m_previousKeySequence;
    }

protected slots:
    void slotKeyPressed(const QKeySequence& keySequence);

protected:
    QKeySequence m_nextKeySequence;
    QKeySequence m_previousKeySequence;

    /*********************************************************************
     * External Input
     *********************************************************************/
public:
    /** Input universe/channel for skipping to the next cue */
    void setNextInputSource(quint32 universe, quint32 channel);
    quint32 nextInputUniverse() const {
        return m_nextInputUniverse;
    }
    quint32 nextInputChannel() const {
        return m_nextInputChannel;
    }


    /** Input universe/channel for skipping to the previous cue */
    void setPreviousInputSource(quint32 universe, quint32 channel);
    quint32 previousInputUniverse() const {
        return m_previousInputUniverse;
    }
    quint32 previousInputChannel() const {
        return m_previousInputChannel;
    }

protected slots:
    void slotNextInputValueChanged(quint32 universe, quint32 channel, uchar value);
    void slotPreviousInputValueChanged(quint32 universe, quint32 channel, uchar value);

private:
    quint32 m_nextInputUniverse;
    quint32 m_nextInputChannel;
    quint32 m_nextLatestValue;

    quint32 m_previousInputUniverse;
    quint32 m_previousInputChannel;
    quint32 m_previousLatestValue;

    /*********************************************************************
     * Caption
     *********************************************************************/
public:
    /** See VCWidget::setCaption() */
    void setCaption(const QString& text);

    /*********************************************************************
     * QLC Mode
     *********************************************************************/
public:
    /** See VCWidget::setCaption() */
    void slotModeChanged(Doc::Mode mode);

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /** Edit properties for this widget */
    void editProperties();

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    static bool loader(const QDomElement* root, QWidget* parent);
    bool loadXML(const QDomElement* root);
    bool saveXML(QDomDocument* doc, QDomElement* vc_root);
};

#endif

