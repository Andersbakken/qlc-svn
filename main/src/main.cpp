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

int parseArgs(int argc, char **argv)
{
  int ret = 0;

  if (argc < 2)
    {
      // No args
      return 0;
    }
  else
    {
      for (int i = 1; i < argc; i++)
	{
	  if (strcmp(argv[i], "-h") == 0 || 
	      strcmp(argv[i], "--help") == 0)
	    {
	      printf("\nUsage:");
	      printf("\n%s [options]", argv[0]);
	      printf("\n\nOptions:");
	      printf("\n-h or --help      Print this help");
	      printf("\n-v or --version   Print version information");
	      
	      ret = -1;
	    }
	  else if (strcmp(argv[i], "-v") == 0 ||
		   strcmp(argv[i], "--version") == 0)
	    {
	      printf("\n%s", IDS_APP_VERSION_STR);
	      ret = -1;
	    }
	}

      // Print a couple of enters
      printf("\n\n");
    }

  return ret;
}

int main(int argc, char **argv)
{
  int result;

  printf("--- Q Light Controller 2 ---\n");
  printf("This program is licensed under the terms of the GNU General Public License.\n");
  printf("Copyright (c) Heikki Junnila (hjunnila@iki.fi)\n");

  if (parseArgs(argc, argv) == -1)
    {
      exit(0);
    }

  // Initialize QApplication object
  QApplication a(argc, argv);
  _qapp = &a;

  // Get widget style from settings
  Settings* settings;
  settings = new Settings();
  settings->load();

  // Allow the usage of the global system style setting
  QStyle* style = settings->style(settings->widgetStyle());
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
