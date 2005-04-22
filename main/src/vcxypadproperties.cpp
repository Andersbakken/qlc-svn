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

#include "vcxypad.h"
#include "uic_vcxypadproperties.h"
#include "vcxypadproperties.h"
#include "vcxypadchannelproperties.h"
#include "devicelist.h"
#include "deviceclass.h"
#include "logicalchannel.h"
#include "device.h"

VCXYPadProperties::VCXYPadProperties(QWidget* parent, const char* name)
                  : UI_VCXYPadProperties(parent, name, true)
{
  assert(parent);
  m_parent = static_cast<VCXYPad*> (parent);

}


VCXYPadProperties::~VCXYPadProperties()
{
}

   
void VCXYPadProperties::init()
{
   m_listX->setAllColumnsShowFocus(true);
   m_listX->setColumnWidthMode(Kid,QListView::Manual);
   m_listX->setColumnWidth(Kid,0);
   m_listY->setAllColumnsShowFocus(true);
   m_listY->setColumnWidthMode(Kid,QListView::Manual);
   m_listY->setColumnWidth(Kid,0);

   fillIt(m_listX, *m_parent->channelsX());
   fillIt(m_listY, *m_parent->channelsY());
}


 void VCXYPadProperties::fillIt(QListView *list, QPtrList<XYChannelUnit> channels)
 {
   QPtrListIterator<XYChannelUnit> it( channels );

   XYChannelUnit *e;

    while ( (e = it.current()) != 0 ) {
       ++it;
       QString t;

       QListViewItem *newItem =  new QListViewItem(list);
       t.sprintf("%d",e->device()->address()+e->channel());
       newItem->setText(Kdmx, t);

       newItem->setText(Kdevice, e->device()->name());
       t.sprintf("%d",e->channel());
       newItem->setText(Kchannel, t);

       newItem->setText(Kcapability, e->device()->deviceClass()->channels()->at(e->channel()-1)->name());
       t.sprintf("%d",e->hi());
       newItem->setText(Kmax, t);
       t.sprintf("%d",e->lo());
       newItem->setText(Kmin, t);

       if(e->reverse())
          newItem->setText(Kreverse, "Yes");
	else
          newItem->setText(Kreverse, "No");
       t.sprintf("%d",e->device()->id());
       newItem->setText(Kid, t);
    }


}


void VCXYPadProperties::slotAdd(QListView *list)
{
    DeviceList* dl = new DeviceList(this);
	dl->init();
	if (dl->exec() == QDialog::Accepted)
	  {
	     QListViewItem *newItem =  new QListViewItem(list);
	     newItem->setText(Kdmx, dl->listView()->currentItem()->text(3));
	     newItem->setText(Kdevice, dl->listView()->currentItem()->text(0)+ " ");
	     newItem->setText(Kchannel, dl->listView()->currentItem()->text(1));
	     newItem->setText(Kcapability, dl->listView()->currentItem()->text(2));
	     newItem->setText(Kmax, "255");
	     newItem->setText(Kmin, "000");
	     newItem->setText(Kreverse, "No");
	     newItem->setText(Kid, dl->listView()->currentItem()->text(4));
	   // _app->doc()->setModified(true);
	  }
	delete dl;

}
void VCXYPadProperties::slotAddY()
{
    slotAdd(m_listY);
}

void VCXYPadProperties::slotAddX()
{
    slotAdd(m_listX);
}

void VCXYPadProperties::slotEditX()
{
    if(m_listX->currentItem())
       slotEdit(m_listX);
}
void VCXYPadProperties::slotEditY()
{
    if(m_listY->currentItem())
       slotEdit(m_listY);
}


void VCXYPadProperties::slotEdit(QListView *list)
{
        VCXYPadChannelProperties* cp = new VCXYPadChannelProperties(this);
	cp->init();
	QListViewItem *newItem =  new QListViewItem(cp->listView());
	newItem->setText(0,list->currentItem()->text(Kdevice));
	newItem->setText(1,list->currentItem()->text(Kchannel));
        newItem->setText(2,list->currentItem()->text(Kcapability));
        newItem->setText(3,list->currentItem()->text(Kdmx));
        cp->listView()->setAllColumnsShowFocus(true);
	cp->m_min->setValue(list->currentItem()->text(Kmin).toInt());
	cp->m_max->setValue(list->currentItem()->text(Kmax).toInt());
	if(list->currentItem()->text(Kreverse) == QString("No"))
               cp->m_reverse->setChecked(false);
	   else
	       cp->m_reverse->setChecked(true);

	if (cp->exec() == QDialog::Accepted)
	  {
	     QString t;
	     t.sprintf("%d",cp->m_min->value());
             list->currentItem()->setText(Kmin, t);
             t.sprintf("%d",cp->m_max->value());
             list->currentItem()->setText(Kmax, t);
	     if(cp->m_reverse->isChecked())
                  list->currentItem()->setText(Kreverse,"Yes");
		else
		   list->currentItem()->setText(Kreverse,"No");
	  }
}


void VCXYPadProperties::slotRemoveX()
{
   delete(m_listX->currentItem());
}

void VCXYPadProperties::slotRemoveY()
{
   delete(m_listY->currentItem());
}










