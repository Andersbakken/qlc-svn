/*
  Q Light Controller
  main.cpp
  
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

#include <qapplication.h>
#include <qstring.h>

#include "settings.h"
#include "app.h"

App* _app;
QApplication* _qapp;

int main(int argc, char *argv[])
{
  int result;
  QApplication a(argc, argv);
  _qapp = &a;

  printf("--- Starting Q Light Controller 2 ---\n");
  printf("This program is licensed under the terms of the GNU General Public License.\n");
  printf("Copyright (c) 2000, 2001, 2002 Heikki Junnila (hjunnila@iki.fi)\n\n");

  // Get widget style from settings
  Settings* settings;
  settings = new Settings();
  settings->load();

  // Allow the usage of the global system style setting
  QStyle* style = settings->getStyle();
  if (style != NULL)
    {
      a.setStyle(style);
    }

  delete settings;

  _app = new App();
  _app->initView();
  a.setMainWidget(_app);

  _app->setCaption("Q Light Controller");
  _app->show();
  
  result = a.exec();

  delete _app;

  return result;
}
