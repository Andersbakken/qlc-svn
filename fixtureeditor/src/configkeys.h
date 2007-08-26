/*
  Q Light Controller
  configkeys.h
  
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

#ifndef CONFIGKEYS_H
#define CONFIGKEYS_H

#include <qstring.h>

// General settings
const QString KEY_OUTPUT_PLUGIN        (       "OutputPlugin" );
const QString KEY_INPUT_PLUGIN         (        "InputPlugin" );
const QString KEY_WIDGET_STYLE         (        "WidgetStyle" );
const QString KEY_OPEN_LAST_WORKSPACE  (  "OpenLastWorkspace" );
const QString KEY_LAST_WORKSPACE_NAME  (  "LastWorkspaceName" );

const QString KEY_APP_X                (      "ApplicationRectX" );
const QString KEY_APP_Y                (      "ApplicationRectY" );
const QString KEY_APP_W                (      "ApplicationRectW" );
const QString KEY_APP_H                (      "ApplicationRectH" );
const QString KEY_APP_MAXIMIZED        (  "ApplicationMaximized" );
const QString KEY_APP_BACKGROUND       ( "ApplicationBackground" );
const QString KEY_APP_SHOW_TOOLTIPS    (          "ShowTooltips" );

const QString KEY_DEVICE_MANAGER_OPEN  (  "DeviceManagerOpen" );
const QString KEY_DEVICE_MANAGER_X     ( "DeviceManagerRectX" );
const QString KEY_DEVICE_MANAGER_Y     ( "DeviceManagerRectY" );
const QString KEY_DEVICE_MANAGER_W     ( "DeviceManagerRectW" );
const QString KEY_DEVICE_MANAGER_H     ( "DeviceManagerRectH" );
const QString KEY_DEVICE_MANAGER_SPLITTER ( "DeviceManagerSplitter" );

// These aren't actually settings keys, but I wanted to get rid of
// the compile-time defines, so I moved them here.
const QString CONFIGFILE               (           "qlc.conf" );
const QString QLCUSERDIR               (               ".qlc" );

#endif
