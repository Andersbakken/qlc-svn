/*
  Q Light Controller
  dmxmaepditor.h
  
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

#ifndef DMXMAPEDITOR_H
#define DMXMAPEDITOR_H

#include "uic_dmxmapeditor.h"

class DMXMap;

class DMXMapEditor : public UI_DMXMapEditor
{
	Q_OBJECT
public:
	DMXMapEditor(QWidget* parent, DMXMap* dmxMap);
	~DMXMapEditor();

	void init();

	/*********************************************************************
	 *
	 *********************************************************************/
protected slots:
	/**
	 * Slot for edit button clicks
	 */
	void slotEditMappingButtonClicked();

	/**
	 * Slot for context menu requests (RMB clicks) on the mapping list view
	 *
	 * Since each selectable output has to have unique menu ID, the IDs
	 * are organized by hundreds:
	 *
	 * The 1st plugin has ID 0
	 * The 1st plugin's outputs can get IDs 0-99
	 * The 2nd plugin has ID 100
	 * The 2nd plugin's outputs can get IDs 100-199
	 * etc.
	 *
	 * It is VERY unlikely that any plugin would have even ten outputs,
	 * but since this method has no serious disadvantages, I suppose being
	 * a bit future-proof wouldn't hurt anyone.
	 *
	 * @param item The clicked QListViewItem*
	 * @param point The exact clicking point
	 * @param column The clicked column
	 */
	void slotListViewContextMenuRequested(QListViewItem* item,
					      const QPoint& point,
					      int column);

	/**
	 * Slot for context menu item activation signals. See the above
	 * function (@ref slotListViewContextMenuRequested) for how the
	 * menu IDs are organized.
	 *
	 * @param item The menu ID of the activated item
	 */
	void slotPluginMenuActivated(int item);

	/*********************************************************************
	 * OK & Cancel
	 *********************************************************************/
public:
	/** Slot for OK button */
	void accept();

protected:
	/** The DMXMap that we're editing here */
	DMXMap* m_dmxMap;

	/** List of available plugin names */
	QStringList m_pluginList;
};

#endif
