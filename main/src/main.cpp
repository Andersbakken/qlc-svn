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

#include "settings.h"
#include "sequencetimer.h"
#include "app.h"

#include <X11/Xlib.h>

App* _app;
QApplication* _qapp;

int parseArgs(int argc, char **argv);

int main(int argc, char **argv)
{
  int result;
  uid_t uid;
  Display* xd;
  XKeyboardState xkbstate;

  /* Get keyboard state (just for key repeat status) */
  xd = XOpenDisplay(NULL);
  ASSERT(xd != NULL);
  XGetKeyboardControl(xd, &xkbstate);
  XCloseDisplay(xd);


  qDebug("--- Q Light Controller %s ---\n", IDS_APP_VERSION_STR);
  qDebug("This program is licensed under the terms of the GNU General Public License.");
  qDebug("Copyright (c) Heikki Junnila (hjunnila@iki.fi)");

  uid = ::getuid();
  if (uid == 0)
    {
      qDebug("\nDo not run QLC as root. Instead, make the executable suid root:");
      qDebug("    chown root.root qlc ; chmod +s qlc\n");
      qDebug("Exit.");
 //     return -1;
    }

  if (parseArgs(argc, argv) == 1)
    {
      return 0;
    }

  SequenceTimer* timer = new SequenceTimer();
  timer->init();

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
  _app->setSequenceTimer(timer);
  _app->initView();
  a.setMainWidget(_app);

  _app->setCaption("Q Light Controller 2");
  _app->show();
  
  result = a.exec();

  delete _app;

  /* Set auto repeat on/off depending on its original state
   * before starting qlc. Virtual Console sets auto repeat off
   * each time it is put to "Operate" mode and on again when it is
   * put to "Design" mode.
   */
  Display* display;
  display = XOpenDisplay(NULL);
  ASSERT(display != NULL);
  if (xkbstate.global_auto_repeat == 0)
    {
      XAutoRepeatOff(display);
    }
  else
    {
      XAutoRepeatOn(display);
    }
  XCloseDisplay(display);

  delete timer;

  return result;
}

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
	  if (::strcmp(argv[i], "-h") == 0 || 
	      ::strcmp(argv[i], "--help") == 0)
	    {
	      qDebug("Usage:");
	      qDebug("%s [options]", argv[0]);
	      qDebug("\nOptions:");
	      qDebug("-h or --help      Print this help");
	      qDebug("-v or --version   Print version information");
	      
	      ret = 1;
	    }
	  else if (::strcmp(argv[i], "-v") == 0 ||
		   ::strcmp(argv[i], "--version") == 0)
	    {
	      qDebug("%s", IDS_APP_VERSION_STR);
	      ret = 1;
	    }
	  else
	    {
	      ret = 0;
	    }
	}

      // Print a couple of enters
      qDebug("\n");
    }

  return ret;
}
