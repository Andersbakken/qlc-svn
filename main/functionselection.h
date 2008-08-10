/*
  Q Light Controller
  functionselection.h

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

#ifndef FUNCTIONSELECTION_H
#define FUNCTIONSELECTION_H

#include <QDialog>
#include <QList>

#include "common/qlctypes.h"
#include "ui_functionselection.h"
#include "function.h"

class QTreeWidgetItem;
class QWidget;

class Fixture;
class Doc;

class FunctionSelection : public QDialog, public Ui_FunctionSelection
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/**
	 * Constructor
	 *
	 * @param parent The parent widget
	 * @param multiple Set true to enable multiple selection
	 * @param disableID A function ID to disable (when adding steps to
	 *                  a chaser, disable the chaser itself)
	 * @param filter Show only functions of the given types OR'd. Use
	 *               Function::Undefined to show all.
	 * @param constFilter If true, don't allow user filter selection
	 */
	FunctionSelection(QWidget* parent,
			  bool multiple,
			  t_function_id disableFunction = KNoID,
			  int filter = Function::Undefined,
			  bool constFilter = false);

	/**
	 * Destructor
	 */
	~FunctionSelection();

	/**
	 * List of selected function IDs
	 */
	QList <t_function_id> selection;

	/*********************************************************************
	 * Internal
	 *********************************************************************/
protected:
	/**
	 * Clear & (re)fill the tree
	 */
	void refillTree();

	/**
	 * Find a top-level item that matches the given fixture instance or
	 * create one if it doesn't exist.
	 *
	 * @param fxi_id The fixture ID to search for
	 * @param doc A QLC Doc* pointer that contains all fixture instances
	 */
	QTreeWidgetItem* fixtureItem(t_fixture_id fxi_id, Doc* doc);

protected slots:
	/**
	 * Item double clicks
	 */
	void slotItemDoubleClicked(QTreeWidgetItem* item);

	void slotCollectionChecked(bool state);
	void slotEFXChecked(bool state);
	void slotChaserChecked(bool state);
	void slotSceneChecked(bool state);

	/**
	 * OK button click
	 */
	void accept();

protected:
	int m_filter;
	t_function_id m_disable;
};

#endif
