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
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

#include "app.h"

#include <X11/Xlib.h>

App* _app;
QApplication* _qapp;
QString openFile;

/*
 * Parse command line arguments
 */
int parseArgs(int argc, char **argv)
{
  int ret = 0;

  if (argc < 2)
    {
      // No args
      ret = 0;
    }
  else
    {
      for (int i = 1; i < argc; i++)
	{
	  if (::strcmp(argv[i], "-v") == -0 ||
		   ::strcmp(argv[i], "--version") == 0)
	    {
	      qDebug(KApplicationVersion);
	      ret = 1;
	    }
	  else if (::strcmp(argv[i], "-o") == 0 ||
		   ::strcmp(argv[i], "--open") == 0)
            {
	       openFile = argv[i+1];
               qDebug(QString("Trying to open: ")+openFile);
            }
          /* User asks for help or has given an unknown option */
          else if (::strcmp(argv[i], "-h") == 0 ||
              ::strcmp(argv[i], "--help") == 0)
            {
              qDebug(" ");
              qDebug("Usage:");
              qDebug("%s [options]", argv[0]);
              qDebug("\nOptions:");
              qDebug("-o file or --open file    open the specified workspace file");
              qDebug("-h or --help              Print this help");
              qDebug("-v or --version           Print version information");

              ret = 1;
            }

	  else
	    {
	      ret = 0;
	    }
	}
    }

  return ret;
}


/*
 * main; entry point for program
 */
int main(int argc, char **argv)
{
  int result = 0;
  openFile = "";
  //
  // Parse any command line arguments
  if (parseArgs(argc, argv) == 1)
    {
      return 0;
    }

  qDebug("* " + KApplicationNameLong + " " + KApplicationVersion);
  qDebug("* This program is licensed under the terms of the GNU GPL.");
  qDebug("* Copyright (c) Heikki Junnila (hjunnila@iki.fi)");

  //
  // Initialize QApplication object
  QApplication a(argc, argv);
  _qapp = &a;

  //
  // Construct the main application class
  _app = new App();
  _app->setCaption(KApplicationNameLong);
  _app->init(openFile);
  a.setMainWidget(_app);
  _app->show();
  
  //
  // Main application loop
  //
  result = a.exec();

  //
  // Delete main app class
  delete _app;

  //
  // Set key repeat on in case QLC is set to turn it off in operate mode.
  // It's pretty safe to assume that most users have it always turned on.
  //
  Display* display = XOpenDisplay(NULL);
  ASSERT(display != NULL);
  XAutoRepeatOn(display);
  XCloseDisplay(display);

  return result;
}
