/*
  Q Light Controller
  vcxypad.h
  
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

#ifndef VCXYPAD_H
#define VCXYPAD_H

#include <qframe.h>
#include <qptrlist.h>

#include "types.h"
#include "device.h"

class QFile;
class QString;
class QPaintEvent;
class QMouseEvent;


class XYChannelUnit
{
public:
        XYChannelUnit();
	XYChannelUnit( Device* device, const t_channel channel,
	             const t_value lo,const t_value hi,const bool reverse);
	~XYChannelUnit();

	t_value m_lo;
	t_value m_hi;
	Device* m_device;
	t_channel m_channel;
	bool m_reverse;

	t_value lo() {return m_lo;}
	t_value hi() {return m_hi;}
	Device* device() {return m_device;}
	t_channel channel(){return m_channel;}
	bool reverse()  {return m_reverse;}

};


class VCXYPad : public QFrame
{
  Q_OBJECT

 public:
  VCXYPad(QWidget* parent);
  virtual ~VCXYPad();

  QPtrList<XYChannelUnit> m_channelsX;
  QPtrList<XYChannelUnit> m_channelsY;
  
  enum ButtonBehaviour
    {
      Normal = 0,
      Exclusive = 1
    };
  
  void init();

  t_vc_id id() const { return m_id; }

  void saveFramesToFile(QFile& file, t_vc_id parentID = 0);
  void saveChildrenToFile(QFile& file);
  virtual void createContents(QPtrList <QString> &list);

  void setBottomFrame(bool set = true);
  bool isBottomFrame() { return m_bottomFrame; }

  void setButtonBehaviour(ButtonBehaviour);
  ButtonBehaviour buttonBehaviour() { return m_buttonBehaviour; }

  QPtrList<XYChannelUnit>* channelsX(){return &m_channelsX;}
  QPtrList<XYChannelUnit>* channelsY(){return &m_channelsY;}

 private slots:
  void slotModeChanged();

 signals:
  void backgroundChanged();

 protected:
  void setID(t_vc_id id);
  void invokeMenu(QPoint point);
  void parseWidgetMenu(int item);

  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void paintEvent(QPaintEvent* e);
  void customEvent(QCustomEvent* e);

  void resizeTo(QPoint p);
  void moveTo(QPoint p);
  void outputDMX(int x, int y);

 protected:
  int m_origX;
  int m_origY;
  int m_xpos;
  int m_ypos;

  t_vc_id m_id;

  bool m_resizeMode;
  bool m_bottomFrame;

  ButtonBehaviour m_buttonBehaviour;

 private:
  static t_vc_id s_nextVCID;

 public:
  static void ResetID() { s_nextVCID = KVCIDMin; }

};

#endif
