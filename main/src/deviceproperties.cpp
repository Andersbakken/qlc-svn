/*
  Q Light Controller
  deviceproperties.cpp
  
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

#include "deviceproperties.h"

DeviceProperties::DeviceProperties(QWidget *parent, const char *name) 
  : QWidget(parent,name) 
{
  initDialog(); 
} 

DeviceProperties::~DeviceProperties()
{
}


void  DeviceProperties::initDialog()
{
  resize(390, 500);
  setMinimumSize(390, 500);
  setMaximumSize(390, 500);
  m_name= new QLineEdit(this,"NoName");
  m_name->setGeometry(80,10,300,30);
  m_name->setMinimumSize(0,0);
  m_name->setText("");

  QLabel_1= new QLabel(this,"NoName");
  QLabel_1->setGeometry(10,10,70,30);
  QLabel_1->setMinimumSize(0,0);
  QLabel_1->setText("Name");

  QLabel_2= new QLabel(this,"NoName");
  QLabel_2->setGeometry(210,50,70,30);
  QLabel_2->setMinimumSize(0,0);
  QLabel_2->setText("Protocol");

  m_protocol= new QLineEdit(this,"NoName");
  m_protocol->setGeometry(280,50,100,30);
  m_protocol->setMinimumSize(0,0);
  m_protocol->setText("");

  QLabel_3= new QLabel(this,"NoName");
  QLabel_3->setGeometry(10,50,70,30);
  QLabel_3->setMinimumSize(0,0);
  QLabel_3->setText("Address");

  m_address= new QSpinBox(this,"NoName");
  m_address->setGeometry(80,50,110,30);
  m_address->setMinimumSize(0,0);
  m_address->setRange(1,512);
  m_address->setValue(1);

  QLabel_4= new QLabel(this,"NoName");
  QLabel_4->setGeometry(10,90,70,30);
  QLabel_4->setMinimumSize(0,0);
  QLabel_4->setText("Channels");

  m_channels= new QSpinBox(this,"NoName");
  m_channels->setGeometry(80,90,110,30);
  m_channels->setMinimumSize(0,0);
  m_channels->setRange(1,512);
  m_channels->setValue(1);

  m_cancel= new QPushButton(this,"NoName");
  m_cancel->setGeometry(280,460,100,30);
  m_cancel->setMinimumSize(0,0);
  m_cancel->setText("Cancel");

  m_ok= new QPushButton(this,"NoName");
  m_ok->setGeometry(170,460,100,30);
  m_ok->setMinimumSize(0,0);
  m_ok->setText("OK");

  m_sequences= new QListView(this,"NoName");
  m_sequences->setGeometry(80,130,190,150);
  m_sequences->setMinimumSize(0,0);
  m_sequences->addColumn("ID");
  m_sequences->addColumn("Name");

  m_scenes= new QListView(this,"NoName");
  m_scenes->setGeometry(80,290,190,150);
  m_scenes->setMinimumSize(0,0);
  m_scenes->addColumn("ID");
  m_scenes->addColumn("Name");

  m_sceneDel= new QPushButton(this,"NoName");
  m_sceneDel->setGeometry(280,400,50,30);
  m_sceneDel->setMinimumSize(0,0);
  m_sceneDel->setText("Del");

  m_sceneGo= new QPushButton(this,"NoName");
  m_sceneGo->setGeometry(280,300,50,30);
  m_sceneGo->setMinimumSize(0,0);
  m_sceneGo->setText("Go!");

  QLabel_5= new QLabel(this,"NoName");
  QLabel_5->setGeometry(10,290,70,20);
  QLabel_5->setMinimumSize(0,0);
  QLabel_5->setText("Scenes");

  QLabel_6= new QLabel(this,"NoName");
  QLabel_6->setGeometry(10,130,70,20);
  QLabel_6->setMinimumSize(0,0);
  QLabel_6->setText("Sequences");

  m_sceneAdd= new QPushButton(this,"NoName");
  m_sceneAdd->setGeometry(280,370,50,30);
  m_sceneAdd->setMinimumSize(0,0);
  m_sceneAdd->setText("Add");

  m_seqDel= new QPushButton(this,"NoName");
  m_seqDel->setGeometry(280,240,50,30);
  m_seqDel->setMinimumSize(0,0);
  m_seqDel->setText("Del");

  m_seqAdd= new QPushButton(this,"NoName");
  m_seqAdd->setGeometry(280,210,50,30);
  m_seqAdd->setMinimumSize(0,0);
  m_seqAdd->setText("Add");

  m_seqGo= new QPushButton(this,"NoName");
  m_seqGo->setGeometry(280,140,50,30);
  m_seqGo->setMinimumSize(0,0);
  m_seqGo->setText("Go!");

  m_sceneEdit= new QPushButton(this,"NoName");
  m_sceneEdit->setGeometry(280,340,50,30);
  m_sceneEdit->setMinimumSize(0,0);
  m_sceneEdit->setText("Edit");

  m_seqEdit= new QPushButton(this,"NoName");
  m_seqEdit->setGeometry(280,180,50,30);
  m_seqEdit->setMinimumSize(0,0);
  m_seqEdit->setText("Edit");
}
