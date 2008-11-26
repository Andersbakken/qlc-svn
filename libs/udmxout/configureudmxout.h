/*
  Q Light Controller
  configureuDMXout.h

  Copyright (c)	2008, Lutz Hillebrand (ilLUTZminator)

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
#ifndef CONFIGUREUDMXOUT_H
#define CONFIGUREUDMXOUT_H

#include <QDialog>
#include "common/qlctypes.h"

#include "ui_configureudmxout.h"

class uDMXOut;

class Configure_uDMXOut : public QDialog, public Ui_Configure_uDMXOut
{
  Q_OBJECT

 public:
  Configure_uDMXOut(QWidget* parent, uDMXOut* plugin);
  virtual ~Configure_uDMXOut();

  void setDevice(QString deviceName);
  QString device();

  void updateStatus();
  int  firstUniverse();

  int  Channels() ;
  int  Debug() ;

 private slots:
  void slotActivateClicked();

 private:
  uDMXOut* m_plugin;
};

#endif
