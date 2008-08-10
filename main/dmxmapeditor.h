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

#include <QDialog>
#include "ui_dmxmapeditor.h"

class DMXMap;

class DMXMapEditor : public QDialog, public Ui_DMXMapEditor
{
	Q_OBJECT
public:
	DMXMapEditor(QWidget* parent, DMXMap* dmxMap);
	~DMXMapEditor();

	/*********************************************************************
	 *
	 *********************************************************************/
protected slots:
	/**
	 * Slot for edit button clicks
	 */
	void slotEditClicked();

	/**
	 * Slot for context menu requests (RMB clicks) on the mapping view
	 *
	 * @param point The exact clicking point
	 */
	void slotContextMenuRequested(const QPoint& point);

	/*********************************************************************
	 * OK & Cancel
	 *********************************************************************/
protected slots:
	/** Slot for OK button */
	void accept();

protected:
	/** The DMXMap that we're editing here */
	DMXMap* m_dmxMap;

	/** List of available plugin names */
	QStringList m_pluginList;
};

#endif
