/*
  Q Light Controller
  vcxypadproperties.h
  
  Copyright (C) 2005, Stefan Krumm, Heikki Junnila
  
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

#ifndef VCXYPADROPERTIES_H
#define VCXYPADROPERTIES_H

#include "types.h"
#include "uic_vcxypadproperties.h"

class VCXYPad;
class XYChannelUnit;

class VCXYPadProperties : public UI_VCXYPadProperties
{
	Q_OBJECT

public:
	VCXYPadProperties(QWidget* parent, const char* name = 0);
	~VCXYPadProperties();

	void init();
    
protected:
	/**
	* Fill a channel list with XYChannelUnit objects
	*/
	void fillChannelList(QListView *list, 
			     QPtrList<XYChannelUnit>* channels);

	/**
	* Create a channel entry to the given parent listview
	*/
	QListViewItem* createChannelEntry(QListView* parent,
					t_device_id deviceID,
					t_channel channel,
					t_value lo,
					t_value hi,
					bool reverse);
	/**
	* Create an XY channel unit from the given list item
	*/
	XYChannelUnit* createChannelUnit(QListViewItem* item);

	/**
	* Display a DMX value menu, divided into submenus of 16 values
	*/
	int invokeDMXValueMenu(const QPoint &point);

protected slots:
	void slotAddX();
	void slotRemoveX();
	void slotAddY();
	void slotRemoveY();
	void slotAdd(QListView *list);

	void slotMaxXChanged(const QString& text);
	void slotMinXChanged(const QString& text);
	void slotMaxYChanged(const QString& text);
	void slotMinYChanged(const QString& text);

	void slotReverseXActivated(const QString& text);
	void slotReverseYActivated(const QString& text);

	void slotSelectionXChanged(QListViewItem* item);
	void slotSelectionYChanged(QListViewItem* item);

	void slotContextMenuRequested(QListViewItem* item,
				      const QPoint &point, int column);

	void slotOKClicked();

protected:
	VCXYPad* m_parent;
};

#endif
