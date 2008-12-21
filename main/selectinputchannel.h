/*
  Q Light Controller
  selectinputchannel.h

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

#ifndef SELECTINPUTCHANNEL_H
#define SELECTINPUTCHANNEL_H

#include <QDialog>
#include <common/qlctypes.h>
#include "ui_selectinputchannel.h"

class QTreeWidgetItem;
class QLCInputChannel;
class InputPatch;

class SelectInputChannel : public QDialog, public Ui_SelectInputChannel
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	SelectInputChannel(QWidget* parent);
	~SelectInputChannel();

	/** Get the selected universe */
	t_input_universe universe() const { return m_universe; }

	/** Get the selected channel within the selected universe */
	t_input_channel channel() const { return m_channel; }

protected slots:
	void accept();

protected:
	t_input_universe m_universe;
	t_input_channel m_channel;

	/********************************************************************
	 * Tree widget
	 ********************************************************************/
protected:
	/** Fill the tree with available input universes & channels */
	void fillTree();
	
	/** Update the contents of a universe item */
	void updateUniverseItem(QTreeWidgetItem* item,
				t_input_universe uni,
				InputPatch* patch);

	/** Update the contents of a channel item */
	void updateChannelItem(QTreeWidgetItem* item,
			       t_input_universe universe,
			       QLCInputChannel* channel);

protected slots:
	/** Receives changed information for items with manual input enabled */
	void slotItemChanged(QTreeWidgetItem* item, int column);
};

#endif

