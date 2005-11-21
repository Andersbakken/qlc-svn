/*
  Q Light Controller
  vcbuttonproperties.h
  
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

#include <assert.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qpushbutton.h>

#include "vcxypad.h"
#include "xychannelunit.h"
#include "vcxypadproperties.h"
#include "devicelist.h"
#include "deviceclass.h"
#include "logicalchannel.h"
#include "device.h"
#include "settings.h"
#include "configkeys.h"
#include "app.h"
#include "doc.h"

extern App* _app;

const int KColumnDeviceName    ( 0 );
const int KColumnChannelName   ( 1 );
const int KColumnLo            ( 2 );
const int KColumnHi            ( 3 );
const int KColumnReverse       ( 4 );
const int KColumnDeviceID      ( 5 );
const int KColumnChannelNumber ( 6 );

const int KComboItemReverse    ( 0 );
const int KComboItemNormal     ( 1 );

VCXYPadProperties::VCXYPadProperties(QWidget* parent, const char* name)
	: UI_VCXYPadProperties(parent, name, true)
{
	m_parent = static_cast<VCXYPad*> (parent);
}


VCXYPadProperties::~VCXYPadProperties()
{
}

   
void VCXYPadProperties::init()
{
	QString dir;
	_app->settings()->get(KEY_SYSTEM_DIR, dir);
	dir += QString("/") + PIXMAPPATH;
	
	m_addX->setPixmap(QPixmap(dir + "/add.xpm"));
	m_addY->setPixmap(QPixmap(dir + "/add.xpm"));

	m_removeX->setPixmap(QPixmap(dir + "/remove.xpm"));
	m_removeY->setPixmap(QPixmap(dir + "/remove.xpm"));
		
	fillChannelList(m_listX, m_parent->channelsX());
	fillChannelList(m_listY, m_parent->channelsY());
	
	m_reverseXCombo->insertItem(Settings::trueValue(), KComboItemReverse);
	m_reverseXCombo->insertItem(Settings::falseValue(), KComboItemNormal);
	
	m_reverseYCombo->insertItem(Settings::trueValue(), KComboItemReverse);
	m_reverseYCombo->insertItem(Settings::falseValue(), KComboItemNormal);
	
	m_listX->setSelected(m_listX->firstChild(), true);
	slotSelectionXChanged(m_listX->firstChild());
	
	m_listY->setSelected(m_listY->firstChild(), true);
	slotSelectionYChanged(m_listY->firstChild());
}

/**
 * Fill a channel list with XYChannelUnit objects
 */
void VCXYPadProperties::fillChannelList(QListView *list, 
					QPtrList<XYChannelUnit>* channels)
{
	QPtrListIterator<XYChannelUnit> it( *channels );
	XYChannelUnit *e;

	while ( (e = *it) != NULL)
	{
		++it;
		
		createChannelEntry(list, e->deviceID(),
				e->channel(),
				e->lo(),
				e->hi(),
				e->reverse());
	}
}

/**
 * Create a channel entry to the given parent listview
 */
QListViewItem* VCXYPadProperties::createChannelEntry(QListView* parent,
						t_device_id deviceID,
						t_channel channel,
						t_value lo,
						t_value hi,
						bool reverse)
{
	Device* device;
	LogicalChannel* log_ch;
	QListViewItem* item;
	QString s;
	
	device = _app->doc()->device(deviceID);
	if (device == NULL)
	{
		return NULL;
	}

	item = new QListViewItem(parent);
	
	// Device name
	item->setText(KColumnDeviceName, device->name());
				
	// Channel name
	log_ch = device->deviceClass()->channels()->at(channel);
	if (log_ch)
	{
		s.sprintf("%.3d: ", channel + 1);
		s += log_ch->name();
		item->setText(KColumnChannelName, s);
	}
	else
	{
		delete item;
		return NULL;
	}
	
	// High limit
	s.sprintf("%.3d", hi);
	item->setText(KColumnHi, s);
	
	// Low limit
	s.sprintf("%.3d", lo);
	item->setText(KColumnLo, s);
	
	// Reverse
	if (reverse)
	{
		item->setText(KColumnReverse, Settings::trueValue());
	}
	else
	{
		item->setText(KColumnReverse, Settings::falseValue());
	}
	
	// Device ID
	s.setNum(deviceID);
	item->setText(KColumnDeviceID, s);
	
	// Channel number
	s.sprintf("%.3d", channel);
	item->setText(KColumnChannelNumber, s);
	
	return item;
}

void VCXYPadProperties::slotAddY()
{
	slotAdd(m_listY);
}

void VCXYPadProperties::slotAddX()
{
	slotAdd(m_listX);
}

void VCXYPadProperties::slotRemoveX()
{
	delete(m_listX->currentItem());
}

void VCXYPadProperties::slotRemoveY()
{
	delete(m_listY->currentItem());
}

void VCXYPadProperties::slotAdd(QListView *list)
{
	DeviceList* dl = new DeviceList(this);
	dl->init();
	if (list == m_listX)
	{
		dl->setCaption("Add a channel to the list of horizontal axes");
	}
	else
	{
		dl->setCaption("Add a channel to the list of vertical axes");
	}
	
	if (dl->exec() == QDialog::Accepted)
	{
		t_device_id did = dl->selectedDeviceID();
		t_channel ch = dl->selectedChannel();
		
		if (did != KNoID && ch != KChannelInvalid)
		{
			createChannelEntry(list, did, ch,
					KChannelValueMin,
					KChannelValueMax,
					false);
		}
	}
	
	delete dl;
}

void VCXYPadProperties::slotMaxXChanged(const QString& text)
{
	QListViewItem* item = m_listX->currentItem();
	if (item)
	{
		item->setText(KColumnHi, text);
	}
}

void VCXYPadProperties::slotMinXChanged(const QString& text)
{
	QListViewItem* item = m_listX->currentItem();
	if (item)
	{
		item->setText(KColumnLo, text);
	}

}

void VCXYPadProperties::slotMaxYChanged(const QString& text)
{
	QListViewItem* item = m_listY->currentItem();
	if (item)
	{
		item->setText(KColumnHi, text);
	}
}

void VCXYPadProperties::slotMinYChanged(const QString& text)
{
	QListViewItem* item = m_listY->currentItem();
	if (item)
	{
		item->setText(KColumnLo, text);
	}
}

void VCXYPadProperties::slotReverseXActivated(const QString& text)
{
	QListViewItem* item = m_listX->currentItem();
	if (item)
	{
		item->setText(KColumnReverse, text);
	}
}

void VCXYPadProperties::slotReverseYActivated(const QString& text)
{
	QListViewItem* item = m_listY->currentItem();
	if (item)
	{
		item->setText(KColumnReverse, text);
	}
}

void VCXYPadProperties::slotSelectionXChanged(QListViewItem* item)
{
	if (item)
	{
		m_minXSpin->setValue(item->text(KColumnLo).toInt());
		m_maxXSpin->setValue(item->text(KColumnHi).toInt());
		if (item->text(KColumnReverse) == Settings::trueValue())
		{
			m_reverseXCombo->setCurrentItem(KComboItemReverse);
		}
		else
		{
			m_reverseXCombo->setCurrentItem(KComboItemNormal);
		}
	}
}

void VCXYPadProperties::slotSelectionYChanged(QListViewItem* item)
{
	if (item)
	{
		m_minYSpin->setValue(item->text(KColumnLo).toInt());
		m_maxYSpin->setValue(item->text(KColumnHi).toInt());
		if (item->text(KColumnReverse) == Settings::trueValue())
		{
			m_reverseYCombo->setCurrentItem(KComboItemReverse);
		}
		else
		{
			m_reverseYCombo->setCurrentItem(KComboItemNormal);
		}
	}
}

/**
 * Set a value to a min/max/reverse column using a context menu
 */
void VCXYPadProperties::slotContextMenuRequested(QListViewItem* item,
						 const QPoint &point,
						 int column)
{
	int result;
	QString s;
	
	if (column == KColumnLo)
	{
		result = invokeDMXValueMenu(point);
	
		if (result <= KChannelValueMax &&
		result >= KChannelValueMin)
		{
			s.sprintf("%.3d", result);
			item->setText(KColumnLo, s);
			
			slotSelectionXChanged(m_listX->currentItem());
			slotSelectionYChanged(m_listY->currentItem());
		}
	}
	else if (column == KColumnHi)
	{
		result = invokeDMXValueMenu(point);
	
		if (result <= KChannelValueMax &&
		result >= KChannelValueMin)
		{
			s.sprintf("%.3d", result);
			item->setText(KColumnHi, s);
			
			slotSelectionXChanged(m_listX->currentItem());
			slotSelectionYChanged(m_listY->currentItem());
		}
	}
	else if (column == KColumnReverse)
	{
		QPopupMenu* menu = new QPopupMenu();
		menu->insertItem("Reverse", KNoID);
		menu->insertSeparator();
		menu->insertItem(Settings::trueValue(), KComboItemReverse);
		menu->insertItem(Settings::falseValue(), KComboItemNormal);
		
		result = menu->exec(point);
		if (result == KComboItemNormal)
		{
			item->setText(KColumnReverse, Settings::falseValue());
		}
		else if (result == KComboItemReverse)
		{
			item->setText(KColumnReverse, Settings::trueValue());
		}
		
		slotSelectionXChanged(m_listX->currentItem());
		slotSelectionYChanged(m_listY->currentItem());
		
		delete menu;
	}
}

/**
 * Display a DMX value menu, divided into submenus of 16 values
 */
int VCXYPadProperties::invokeDMXValueMenu(const QPoint &point)
{
	int result;
	
	QPopupMenu* menu = new QPopupMenu;
	QPtrList <QPopupMenu> deleteList;

	menu->insertItem("Value", KNoID);
	menu->insertSeparator();

	menu->insertItem(QString::number(KChannelValueMin), KChannelValueMin);
	menu->insertItem(QString::number(KChannelValueMax), KChannelValueMax);
	for (t_value i = 0; i != KChannelValueMax; i += 15)
	{
		QPopupMenu* sub = new QPopupMenu();
		deleteList.append(sub);

		QString top;
		top.sprintf("%d - %d", i+1, i + 15);

		for (t_value j = 1; j < 16; j++)
		{
			QString num;
			num.setNum(i + j);
			sub->insertItem(num, i + j);
		}
      
		menu->insertItem(top, sub);
	}

	result = menu->exec(point);

	while (deleteList.isEmpty() == false)
	{
		delete deleteList.take(0);
	}

	delete menu;	

	return result;
}

void VCXYPadProperties::slotOKClicked()
{	
	QPtrList<XYChannelUnit>* list;

	// Update the X list
	list = m_parent->channelsX();
	
	list->setAutoDelete(true);
	list->clear();
	list->setAutoDelete(false);
	
	QListViewItemIterator xit(m_listX);
	
	while (xit.current())
	{
		list->append(createChannelUnit(*xit));
		++xit;
	}
	
	// Update the Y list
	list = m_parent->channelsY();
	
	list->setAutoDelete(true);
	list->clear();
	list->setAutoDelete(false);
	
	QListViewItemIterator yit(m_listY);
	
	while (yit.current())
	{
		list->append(createChannelUnit(*yit));
		++yit;
	}
 close();
}

/**
 * Create an XY channel unit from the given list item
 */
XYChannelUnit* VCXYPadProperties::createChannelUnit(QListViewItem* item)
{
	if (item == NULL)
	{
		return NULL;
	}
	else
	{
		return new XYChannelUnit(
			item->text(KColumnDeviceID).toInt(),
			item->text(KColumnChannelNumber).toInt(),
			item->text(KColumnLo).toInt(),
			item->text(KColumnHi).toInt(),
			item->text(KColumnReverse) == Settings::trueValue()
		);
	}
}





