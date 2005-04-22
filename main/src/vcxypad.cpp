/*
  Q Light Controller
  vcxypad.cpp
  
  Copyright (C) 2005 Heikki Junnila, Stefan Krumm
  
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

#include "vcxypad.h"
#include "vcbutton.h"
#include "device.h"
#include "vclabel.h"
#include "vcdockslider.h"
#include "floatingedit.h"
#include "app.h"
#include "doc.h"
#include "virtualconsole.h"
#include "settings.h"
#include "../../libs/common/minmax.h"
#include "configkeys.h"
#include "vcxypadproperties.h"

#include <qcursor.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qptrlist.h>
#include <stdio.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qobjectlist.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qlistview.h>


extern App* _app;

t_vc_id VCXYPad::s_nextVCID = KVCIDMin;

const int KFrameStyle      ( QFrame::StyledPanel | QFrame::Sunken );
const int KColorMask       ( 0xff );
const int KMoveThreshold   (    5 ); // Pixels


XYChannelUnit::XYChannelUnit()
{
}


XYChannelUnit::XYChannelUnit( Device* device, const t_channel channel,
	             const t_value lo,const t_value hi,const bool reverse)	      
{
   m_device = device;
   m_channel = channel;
   m_lo = lo;
   m_hi = hi;
   m_reverse = reverse;
}

XYChannelUnit::~XYChannelUnit()
{
}


VCXYPad::VCXYPad(QWidget* parent) 
  : QFrame(parent),
    m_origX            ( 0 ),
    m_origY            ( 0 ),
    m_xpos             ( 0 ),
    m_ypos             ( 0 ),
    m_id               ( s_nextVCID++ ),
    m_resizeMode       ( false ),
    m_bottomFrame      ( false ),
    m_buttonBehaviour  ( Normal )
{
   //m_channelsY = new QPtrList<XYChannelUnit>;
}

VCXYPad::~VCXYPad()
{
}

void VCXYPad::init()
{
  setMinimumSize(20, 20);

  resize(120, 120);
  setFrameStyle(QFrame::Panel | QFrame::Sunken);
  setCursor(Qt::CrossCursor);
  connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
}

void VCXYPad::setBottomFrame(bool set) 
{ 
  m_bottomFrame = set;
  if (set)
    {
      setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    }
  else
    {
      setFrameStyle(QFrame::Panel | QFrame::Sunken);
    }
}

void VCXYPad::setID(t_vc_id newid)
{
  m_id = newid;
  if (newid >= s_nextVCID)
    {
      // Set the biggest id number + 1 to be the next free id
      s_nextVCID = newid + 1;
    }
}

void VCXYPad::setButtonBehaviour(ButtonBehaviour b)
{
  m_buttonBehaviour = b;
}

void VCXYPad::saveFramesToFile(QFile& file, t_vc_id parentID)
{
  QString s;
  QString t;

  // Comment
  s = QString("# Virtual Console Frame Entry\n");
  file.writeBlock((const char*) s, s.length());

  // Entry type
  s = QString("Entry = VCXYPad") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Name
  s = QString("Name = ") + caption() + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Parent ID
  if (parentID != 0)
    {
      t.setNum(parentID);
      s = QString("Parent = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }
    else
    {// No  idea what's happening. Parent must be the VC top frame.
      s = QString("Parent = 1") + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }
  // X Channels and limits
  QPtrListIterator<XYChannelUnit> xit(m_channelsX);
  XYChannelUnit *xyc;
     while ( (xyc = xit.current()) != 0 )
       {
         ++xit;
	 if( xyc->m_reverse == true )
            s.sprintf("ChannelEntryX = %d,%d,%d,%d,true\n",xyc->device()->id(),xyc->channel(),xyc->lo(), xyc->hi());
	 else
            s.sprintf("ChannelEntryX = %d,%d,%d,%d,false\n",xyc->device()->id(),xyc->channel(),xyc->lo(), xyc->hi());
         file.writeBlock((const char*) s, s.length());
       }
  // Y Channels and limits
  QPtrListIterator<XYChannelUnit> yit(m_channelsY);
     while ( (xyc = yit.current()) != 0 )
       {
         ++yit;
	 if( xyc->m_reverse == true )
            s.sprintf("ChannelEntryY = %d,%d,%d,%d,true\n",xyc->device()->id(),xyc->channel(),xyc->lo(), xyc->hi());
	 else
            s.sprintf("ChannelEntryY = %d,%d,%d,%d,false\n",xyc->device()->id(),xyc->channel(),xyc->lo(), xyc->hi());
         file.writeBlock((const char*) s, s.length());
       }
  // Geometry
  if (m_bottomFrame == false)
    {
      // X
      t.setNum(x());
      s = QString("X = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
      
      // Y
      t.setNum(y());
      s = QString("Y = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
      
      // W
      t.setNum(width());
      s = QString("Width = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
      
      // H
      t.setNum(height());
      s = QString("Height = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }

  // Palette
  if (ownPalette())
    {
      // Text color
      t.setNum(qRgb(paletteForegroundColor().red(),
		    paletteForegroundColor().green(),
		    paletteForegroundColor().blue()));
      s = QString("Textcolor = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());

      // Background color
      t.setNum(qRgb(paletteBackgroundColor().red(),
		    paletteBackgroundColor().green(),
		    paletteBackgroundColor().blue()));
      s = QString("Backgroundcolor = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }

  // Background pixmap
  if (paletteBackgroundPixmap())
    {
      s = QString("Pixmap = " + iconText() + QString("\n"));
      file.writeBlock((const char*) s, s.length());
    }

  // Font
  s = QString("Font = ") + font().toString() + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Frame
  if (frameStyle() & KFrameStyle)
    {
      s = QString("Frame = ") + Settings::trueValue() + QString("\n");
    }
  else
    {
      s = QString("Frame = ") + Settings::falseValue() + QString("\n");
    }
  file.writeBlock((const char*) s, s.length());

  // Button Behaviour
  t.setNum(m_buttonBehaviour);
  s = QString("ButtonBehaviour = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // ID
  t.setNum(id());
  s = QString("ID = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  if (children() != NULL)
    {
      QObjectList* ol = (QObjectList*) children();
      QObjectListIt it(*ol);
      
      // Child frames
      for (; it.current() != NULL; ++it)
	{
	  if (QString(it.current()->className()) == QString("VCXYPad"))
	    {
	      VCXYPad* w = (VCXYPad*) it.current();
	      w->saveFramesToFile(file, id());
	    }
	}
    }
}

void VCXYPad::saveChildrenToFile(QFile& file)
{
  if (children() != NULL)
    {
      QObjectList* ol = (QObjectList*) children();
      QObjectListIt it(*ol);
      
      // Child frames
      while(it.current())
	{
	  if (QString(it.current()->className()) == QString("VCButton"))
	    {
	      ((VCButton*) it.current())->saveToFile(file, id());
	    }
	  else if (QString(it.current()->className()) == QString("VCLabel"))
	    {
	      ((VCLabel*) it.current())->saveToFile(file, id());
	    }
	  else if (QString(it.current()->className())==QString("VCDockSlider"))
	    {
	      ((VCDockSlider*) it.current())->saveToFile(file, id());
	    }
	  else if (QString(it.current()->className()) == QString("VCXYPad"))
	    {
	      ((VCXYPad*) it.current())->saveChildrenToFile(file);
	    }

	  ++it;
	}
    }
}

void VCXYPad::createContents(QPtrList <QString> &list)
{
  QRect rect(30, 30, 30, 30);

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("ID"))
	{
	  setID(list.next()->toInt());
	}
      else if (*s == QString("ButtonBehaviour"))
	{
	  setButtonBehaviour(static_cast<ButtonBehaviour>
			     (list.next()->toInt()));
	}
      else if (*s == QString("Parent"))
	{
	  if (m_bottomFrame == false)
	    {
	      VCFrame* parent = 
		_app->virtualConsole()->getFrame(list.next()->toInt());

	      if (parent != NULL)
		{
		  reparent((QWidget*) parent, 0, QPoint(0, 0), true);
		}
	    }
	  else
	    {
	      list.next();
	    }
	}
      else if (*s == QString("Textcolor"))
	{
	  QColor qc;
	  qc.setRgb(list.next()->toUInt());
	  setPaletteForegroundColor(qc);
	}
      else if (*s == QString("Backgroundcolor"))
	{
	  QColor qc;
	  qc.setRgb(list.next()->toUInt());
	  setPaletteBackgroundColor(qc);
	}
      else if (*s == QString("Color"))
	{
	  // Backwards compatibility for frame background color
	  QString t = *(list.next());
	  int i = t.find(QString(","));
	  int r = t.left(i).toInt();
	  int j = t.find(QString(","), i + 1);
	  int g = t.mid(i+1, j-i-1).toInt();
	  int b = t.mid(j+1).toInt();
	  QColor qc(r, g, b);
	  setPaletteBackgroundColor(qc);
	}
      else if (*s == QString("Pixmap"))
	{
	  QString t;
	  t = *(list.next());
	  
	  QPixmap pm(t);
	  if (pm.isNull() == false)
	    {
	      setIconText(t);
	      setPaletteBackgroundPixmap(pm);
	    }
	}
      else if (*s == QString("Font"))
	{
	  QFont f = font();
	  QString q = *(list.next());
	  f.fromString(q);
	  setFont(f);
	}
	else if (*s == QString("ChannelEntryX"))
	{
           QString t = *(list.next());
	   QStringList lst( QStringList::split( ",", t ) );
           bool foundDevice=false;
           for (t_device_id i = 0; i < KDeviceArraySize; i++)
             {
               Device* dev = _app->doc()->device(i);
               if (!dev)
	         {
	            continue;
	         }
                 else
	         {
	           if(dev->id() == lst[0].toInt())
	             {
	                XYChannelUnit * xyc = new XYChannelUnit(dev, lst[1].toInt(),
	                                                             lst[2].toInt(),
								     lst[3].toInt(),
								     lst[4] == "true");
			m_channelsX.append(xyc);
			foundDevice=true;
		     }
                 }
	     }
	     if(!foundDevice)
	        qDebug("Error: couldn't find device# " + lst[0] + " for XY-Pad channel!");
	}
	else if (*s == QString("ChannelEntryY"))
	{
           QString t = *(list.next());
	   QStringList lst( QStringList::split( ",", t ) );
           bool foundDevice=false;
           for (t_device_id i = 0; i < KDeviceArraySize; i++)
             {
               Device* dev = _app->doc()->device(i);
               if (!dev)
	         {
	            continue;
	         }
                 else
	         {
	           if(dev->id() == lst[0].toInt())
	             {
	                XYChannelUnit * xyc = new XYChannelUnit(dev, lst[1].toInt(),
	                                                             lst[2].toInt(),
								     lst[3].toInt(),
								     lst[4] == "true");
			m_channelsY.append(xyc);
			foundDevice=true;
		     }
                 }
	     }
	     if(!foundDevice)
	        qDebug("Error: couldn't find device# " + lst[0] + " for XY-Pad channel!");
	}
      else if (*s == QString("X"))
	{
	  rect.setX(list.next()->toInt());
	}
      else if (*s == QString("Y"))
	{
	  rect.setY(list.next()->toInt());
	}
      else if (*s == QString("Width"))
	{
	  rect.setWidth(list.next()->toInt());
	}
      else if (*s == QString("Height"))
	{
	  rect.setHeight(list.next()->toInt());
	}
      else
	{
	  // Unknown keyword, ignore
	  *list.next();
	}
    }

  if (m_bottomFrame == false)
    {
      setGeometry(rect);
    }
}


void VCXYPad::paintEvent(QPaintEvent* e)
{
  QFrame::paintEvent(e);

  QPainter p(this);

  if (_app->mode() == App::Design && 
      _app->virtualConsole()->selectedWidget() == this &&
      m_bottomFrame == false)
    {
      // Draw a dotted line around the widget
      QPen pen(DotLine);
      pen.setWidth(2);
      p.setPen(pen);
      p.drawRect(1, 1, rect().width() - 1, rect().height() - 1);

      // Draw a resize handle
      QBrush b(SolidPattern);
      p.fillRect(rect().width() - 10, rect().height() - 10, 10, 10, b);
    }
}

void VCXYPad::slotModeChanged()
{
  repaint();
}

void VCXYPad::mousePressEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      _app->virtualConsole()->setSelectedWidget(this);

      if (m_resizeMode == true && m_bottomFrame == false)
	{
	  setMouseTracking(false);
	  m_resizeMode = false;
	}
      
      if ((e->button() & LeftButton || e->button() & MidButton)
	  && m_bottomFrame == false)
	{
	  if (e->x() > rect().width() - 10 &&
	      e->y() > rect().height() - 10)
	    {
	      m_resizeMode = true;
	      setMouseTracking(true);
	      setCursor(QCursor(SizeFDiagCursor));
	    }
	  else
	    {
	      m_origX = e->globalX();
	      m_origY = e->globalY();
	      setCursor(QCursor(SizeAllCursor));
	    }
	}
      else if (e->button() & RightButton)
	{
	  invokeMenu(mapToGlobal(e->pos()));
	}
    }
  else
    {
      setMouseTracking(true);
      setCursor(Qt::CrossCursor);
      outputDMX( e->x(), e->y());
      //QFrame::mousePressEvent(e);
    }
}

void VCXYPad::invokeMenu(QPoint point)
{
  QPopupMenu* menu = new QPopupMenu();
  menu->insertItem("Edit", _app->virtualConsole()->editMenu());
  menu->insertItem("Add", _app->virtualConsole()->addMenu());
  menu->exec(point);
  delete menu;
}

void VCXYPad::parseWidgetMenu(int item)
{
  switch (item)
    {
    case KVCMenuEditProperties:
      {
	VCXYPadProperties* vcfp = new VCXYPadProperties(this);
	vcfp->init();
	if (vcfp->exec() == QDialog::Accepted)
	  {
	   _app->doc()->setModified(true);
	   m_channelsX.clear();
	   QListViewItemIterator it( vcfp->m_listX );
	   // abuse all the listview items as mmemory
           while ( it.current() ) {
	      //serch device
	      Device* dev = new Device();
	      for (t_device_id i = 0; i < KDeviceArraySize; i++)
                {
                  dev = _app->doc()->device(i);
                  if (dev)
	             {

	               if(dev->id() == it.current()->text(Kid).toInt())
		         {
                           m_channelsX.append( new XYChannelUnit(
		           dev,
		           it.current()->text(Kchannel).toInt(),
                           it.current()->text(Kmin).toInt(),
			   it.current()->text(Kmax).toInt(),
		           it.current()->text(Kreverse)=="Yes"));
			 }
		     }
	        }
                 ++it;
	     }
	   m_channelsY.clear();
	   QListViewItemIterator yit( vcfp->m_listY );
	   // abuse all the listview items as mmemory
           while ( yit.current() ) {
	      //serch device
	      Device* dev = new Device();
	      for (t_device_id i = 0; i < KDeviceArraySize; i++)
                {
                  dev = _app->doc()->device(i);
                  if (dev)
	             {

	               if(dev->id() == yit.current()->text(Kid).toInt())
		         {
                           m_channelsY.append( new XYChannelUnit(
		           dev,
		           yit.current()->text(Kchannel).toInt(),
                           yit.current()->text(Kmin).toInt(),
			   yit.current()->text(Kmax).toInt(),
		           yit.current()->text(Kreverse)=="Yes"));
			 }
		     }
	        }
                 ++yit;
	  }
      }
   }


      break;

    case KVCMenuBackgroundFrame:
      {
	if (frameStyle() & KFrameStyle)
	  {
	    setFrameStyle(NoFrame);
	  }
	else
	  {
	    setFrameStyle(KFrameStyle);
	  }
	_app->doc()->setModified(true);
      }
      break;

    default:
      break;
    }
}

void VCXYPad::mouseReleaseEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      //this->setCursor(QCursor(ArrowCursor));
      setCursor(Qt::CrossCursor);
      m_resizeMode = false;
      setMouseTracking(false);
    }
  else
    {
     setMouseTracking(false);
     //unsetCursor();
      QFrame::mouseReleaseEvent(e);
    }
}

void VCXYPad::mouseMoveEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      if (m_resizeMode == true)
	{	  
	  QPoint p(QCursor::pos());
	  resizeTo(mapFromGlobal(p));
	  _app->doc()->setModified(true);
	}
      else if (e->state() & LeftButton || e->state() & MidButton)
	{
	  QPoint p(QCursor::pos());
	  moveTo(parentWidget()->mapFromGlobal(p));
	  _app->doc()->setModified(true);
	}
    }
  else
    { // the following is NOT done by hasMouse() because that fails if
      // there are child widgets   
      if( e->x()>0 &&  e->y()>0  && e->x()<rect().width() && e->y()<rect().height()  )
        {
	   outputDMX( e->x(), e->y());
	   setCursor(Qt::CrossCursor);
	   
	}
      else
        {
	   unsetCursor();
	}	
      QFrame::mouseMoveEvent(e);
    }
}


void VCXYPad::outputDMX(int x, int y)
{
    QPtrListIterator<XYChannelUnit> xit(*channelsX());
    XYChannelUnit *xyc;
    while ( (xyc = xit.current()) != 0 ) {
      ++xit;
      int delta = xyc->hi() - xyc->lo();
      int xx = xyc->lo() + int(delta*x/rect().width());
      if( xyc->reverse() == false)
           _app->outputPlugin()->writeChannel(xyc->device()->address()+xyc->channel()-1,
				     (t_value) xx);
        else
	   _app->outputPlugin()->writeChannel(xyc->device()->address()+xyc->channel()-1,
				     (t_value) 255-xx);
     }


    QPtrListIterator<XYChannelUnit> yit(*channelsY());
    //XYChannelUnit *xyc;
    while ( (xyc = yit.current()) != 0 ) {
      ++yit;
      int delta = xyc->hi() - xyc->lo();
      int xx = xyc->lo() + int(delta*y/rect().height());
      if( xyc->reverse() == false)
           _app->outputPlugin()->writeChannel(xyc->device()->address()+xyc->channel()-1,
				     (t_value) xx);
        else
	   _app->outputPlugin()->writeChannel(xyc->device()->address()+xyc->channel()-1,
				     (t_value) 255-xx);
     }




}


void VCXYPad::customEvent(QCustomEvent* e)
{
  if (e->type() == KVCMenuEvent)
    {
      parseWidgetMenu(((VCMenuEvent*) e)->menuItem());
    }
}


void VCXYPad::resizeTo(QPoint p)
{
  // Grid settings
  if (_app->virtualConsole()->isGridEnabled())
    {
      p.setX(p.x() - (p.x() % _app->virtualConsole()->gridX()));
      p.setY(p.y() - (p.y() % _app->virtualConsole()->gridY()));
    }

  // Map to parent coordinates so that they can be compared
  p = mapToParent(p);

  // Don't move beyond left or right
  if (p.x() < 0)
    {
      p.setX(0);
    }
  else if (p.x() > parentWidget()->width())
    {
      p.setX(parentWidget()->width());
    }
  
  // Don't move beyond top or bottom
  if (p.y() < 0)
    {
      p.setY(0);
    }
  else if (p.y() > parentWidget()->height())
    {
      p.setY(parentWidget()->height());
    }

  // Map back so that this can be resized
  p = mapFromParent(p);

  // Do the resize
  resize(p.x(), p.y());
}


void VCXYPad::moveTo(QPoint p)
{
  // Grid settings
  if (_app->virtualConsole()->isGridEnabled())
    {
      p.setX(p.x() - (p.x() % _app->virtualConsole()->gridX()));
      p.setY(p.y() - (p.y() % _app->virtualConsole()->gridY()));
    }
  
  // Don't move beyond left or right
  if (p.x() < 0)
    {
      p.setX(0);
    }
  else if (p.x() + rect().width() > parentWidget()->width())
    {
      p.setX(parentWidget()->width() - rect().width());
    }
  
  // Don't move beyond top or bottom
  if (p.y() < 0)
    {
      p.setY(0);
    }
  else if (p.y() + rect().height() > parentWidget()->height())
    {
      p.setY(parentWidget()->height() - rect().height());
    }

  // Do the move
  move(p);
}
