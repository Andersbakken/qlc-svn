/*
  Q Light Controller
  deviceproperties.h
  
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

#ifndef DEVICEPROPERTIES_H
#define DEVICEPROPERTIES_H

#include <qwidget.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qwidget.h>

class DeviceProperties : public QWidget
{
   Q_OBJECT
public: 
   DeviceProperties(QWidget *parent=0, const char *name=0);
   ~DeviceProperties();
   
 protected: 
   void initDialog();

   QLineEdit *m_name;
   QLineEdit *m_protocol;
   QLabel *QLabel_1;
   QLabel *QLabel_2;
   QLabel *QLabel_3;
   QLabel *QLabel_4;
   QLabel *QLabel_5;
   QLabel *QLabel_6;
   QSpinBox *m_address;
   QSpinBox *m_channels;
   QPushButton *m_cancel;
   QPushButton *m_ok;
   QListView *m_sequences;
   QListView *m_scenes;
   QPushButton *m_sceneGo;
   QPushButton *m_sceneEdit;
   QPushButton *m_sceneAdd;
   QPushButton *m_sceneDel;
   QPushButton *m_seqGo;
   QPushButton *m_seqEdit;
   QPushButton *m_seqAdd;
   QPushButton *m_seqDel;

 private: 
};

#endif




























































