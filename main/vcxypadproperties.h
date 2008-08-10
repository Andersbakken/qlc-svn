/*
  Q Light Controller
  vcxypadproperties.h
  
  Copyright (c) Stefan Krumm, Heikki Junnila
  
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

#include <QDialog>

#include "common/qlctypes.h"
#include "ui_vcxypadproperties.h"

class VCXYPad;
class XYChannelUnit;

class VCXYPadProperties : public QDialog, public Ui_VCXYPadProperties
{
	Q_OBJECT

public:
	VCXYPadProperties(QWidget* parent, VCXYPad* xypad);
	~VCXYPadProperties();
    
protected:
	/**
	* Fill a channel list with XYChannelUnit objects
	*/
	void fillChannelList(QTreeWidget *list,
			     QList <XYChannelUnit*>* channels);

	/**
	* Create a channel entry to the given parent listview
	*/
	QTreeWidgetItem* createChannelEntry(QTreeWidget* parent,
					    t_fixture_id fixtureID,
					    t_channel channel,
					    t_value lo,
					    t_value hi,
					    bool reverse);
	/**
	* Create an XY channel unit from the given list item
	*/
	XYChannelUnit* createChannelUnit(QTreeWidgetItem* item);

protected:
	void addChannel(QTreeWidget* list);

protected slots:
	void slotAddX();
	void slotRemoveX();
	void slotAddY();
	void slotRemoveY();

	void slotMaxXChanged(const QString& text);
	void slotMinXChanged(const QString& text);
	void slotMaxYChanged(const QString& text);
	void slotMinYChanged(const QString& text);

	void slotReverseXToggled(bool state);
	void slotReverseYToggled(bool state);

	void slotSelectionXChanged();
	void slotSelectionYChanged();

	void accept();

protected:
	VCXYPad* m_xypad;
};

#endif
