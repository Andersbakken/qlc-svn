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

#include "vcwidget.h"
#include "keybind.h"

class QTreeWidgetItem;
class QDomDocument;
class QDomElement;
class QTreeWidget;

class VCCueListProperties;
class Function;

#define KXMLQLCVCCueList "CueList"
#define KXMLQLCVCCueListFunction "Function"

#define KVCCueListColumnNumber 0
#define KVCCueListColumnName   1
#define KVCCueListColumnID     2

class VCCueList : public VCWidget
{
	Q_OBJECT

	friend class VCCueListProperties;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** Normal constructor */
	VCCueList(QWidget* parent);

	/** Destructor */
	~VCCueList();

private:
	/** Prevent copying thru operator= or copy constructor since QObject's
	    parental properties get confused when copied. */
	Q_DISABLE_COPY(VCCueList)

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

	/** Skip to the next cue */
	void slotNextCue();

	/** Slot to catch function stopped signals */
	void slotFunctionStopped(t_function_id fid);

	/** Slot that is called whenever the current item changes (either by
	    pressing the key binding or clicking an item with mouse) */
	void slotItemActivated(QTreeWidgetItem* item);

protected:
	QTreeWidget* m_list;
	Function* m_current;

	/*********************************************************************
	 * Key Bind
	 *********************************************************************/
public:
	void setKeyBind(const KeyBind& kb);
	const KeyBind keyBind() const { return m_keyBind; }

protected:
	KeyBind m_keyBind;

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
	void slotModeChanged(App::Mode mode);

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
	static bool loader(QDomDocument* doc, QDomElement* root,
			   QWidget* parent);
	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool saveXML(QDomDocument* doc, QDomElement* vc_root);
};

#endif

