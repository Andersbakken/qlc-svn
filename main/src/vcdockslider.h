/*
  Q Light Controller
  vcdockslider.h

  Copyright (C) 2000, 2001, 2002 Heikki Junnila

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

#ifndef VCDOCKSLIDER_H
#define VCDOCKSLIDER_H

#include <qptrlist.h>
#include <qstring.h>
#include <qdatetime.h>

#include "uic_vcdockslider.h"
#include "types.h"

class QMouseEvent;
class QFile;
class QPoint;
class SliderKeyBind;

const QString KEY_DEFAULT_FADE_MIN ( "DefaultFadeMin" );
const QString KEY_DEFAULT_FADE_MAX ( "DefaultFadeMax" );
const QString KEY_DEFAULT_HOLD_MIN ( "DefaultHoldMin" );
const QString KEY_DEFAULT_HOLD_MAX ( "DefaultHoldMax" );

class VCDockSlider : public UI_VCDockSlider
{
  Q_OBJECT
    
 public:
  VCDockSlider(QWidget* parent, bool isStatic = false,
	       const char* name = NULL);
  ~VCDockSlider();

  void init();

  void setCaption(const QString&);

  SliderKeyBind* sliderKeyBind() { return m_sliderKeyBind; }
  void setSliderKeyBind(const SliderKeyBind* skb);
  void setChannel(int channel){ m_channel = channel;}
  int channel() const { return m_channel; }

  //
  // Mode stuff
  //
  enum Mode
    {
      Speed  = 0,
      Level = 1,
      Submaster = 2
    };
  
  void setMode(Mode m);
  Mode mode() { return m_mode; }
  QString modeString(Mode mode);
  
  // Bus stuff
  bool setBusID(t_bus_id id);
  t_bus_id busID() const { return m_busID; }
  void setBusRange(t_bus_value lo, t_bus_value hi);
  void busRange(t_bus_value &lo, t_bus_value &hi);

  //
  // Level & Submaster stuff
  //
  QValueList <t_channel>* channels() { return &m_channels; }
  void assignSubmasters(bool assign);
  void setLevelRange(t_value low, t_value hi);
  void levelRange(t_value& lo, t_value& hi);

  void createContents(QPtrList <QString> &list);
  void saveToFile(QFile &file, t_vc_id parentID);

 public slots:
  void pressUp();
  void pressDown();
  void slotFeedBack();
  void slotInputEvent(const int,const int,const int);


 private slots:
  void slotSliderValueChanged(const int);
  void slotTapInButtonClicked();
  void slotModeChanged();
 // void slotInputEvent(int id, int channel, int value);
  
  void slotBusNameChanged(t_bus_id, const QString&);
  void slotBusValueChanged(t_bus_id, t_bus_value);

 protected:
  void invokeMenu(QPoint);
  void parseWidgetMenu(int);

  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mouseDoubleClickEvent(QMouseEvent*);
  void paintEvent(QPaintEvent*);
  void contextMenuEvent(QContextMenuEvent*);
  void customEvent(QCustomEvent*);

  void resizeTo(QPoint);
  void moveTo(QPoint);

 private:
  t_vc_id m_id;
  Mode m_mode;
  t_bus_id m_busID;

  int m_busLowLimit;
  int m_busHighLimit;

  int m_levelLowLimit;
  int m_levelHighLimit;


  int m_channel;

  bool m_static;
  bool m_updateOnly;

  int m_xpos;
  int m_ypos;
  bool m_resizeMode;
  QPoint m_mousePressPoint;
  
  QTime m_time;

  QValueList<t_channel> m_channels;

  SliderKeyBind* m_sliderKeyBind;
};

#endif

