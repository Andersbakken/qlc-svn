/*
  Q Light Controller
  main.cpp

  Copyright (C) Heikki Junnila

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

#include <QApplication>
#include <QTranslator>
#include <QMetaType>
#include <QLocale>
#include <QString>
#include <QDebug>
#include <QHash>

#include <sys/types.h>
#include <unistd.h>

#include "common/qlctypes.h"

#include "app.h"
#include "doc.h"

#ifndef WIN32
#include <X11/Xlib.h>
#endif

void print_version()
{
	qDebug() << KApplicationNameLong << KApplicationVersion;
	qDebug() << "This program is licensed under the terms of GNU GPL.";
	qDebug() << "Copyright (c) Heikki Junnila (hjunnila@users.sf.net)";
}

void print_usage()
{
	print_version();

	qDebug() << "Usage:";
	qDebug() << "  qlc [options]";
	qDebug() << "\nOptions:";
	qDebug() << "  -o <file> or --open <file>    Open the specified workspace file";
	qDebug() << "  -p or --operate               Start in operate mode";
	qDebug() << "  -h or --help                  Print this help";
	qDebug() << "  -v or --version               Print version information";
}

/**
 * Parse command line arguments
 *
 * @param argc Number of arguments in array argv
 * @param argv Arguments array
 *
 * @return true to continue application init; otherwise false
 */
bool parseArgs(App* app, int argc, char **argv)
{
	bool result = true;
	int i = 0;
	QString s;

	Q_ASSERT(app != NULL);

	for (i = 1; i < argc; i++)
	{
		if (::strcmp(argv[i], "-v") == 0 ||
		    ::strcmp(argv[i], "--version") == 0)
		{
			print_version();
			result = false;
		}
		else if (::strcmp(argv[i], "-h") == 0 ||
			 ::strcmp(argv[i], "--help") == 0)
		{
			print_usage();
			result = false;
		}
		else if (::strcmp(argv[i], "-p") == 0 ||
			 ::strcmp(argv[i], "--operate") == 0)
		{
			app->slotModeOperate();
			result = true;
		}
		else if (::strcmp(argv[i], "-o") == 0 ||
			 ::strcmp(argv[i], "--open") == 0)
		{
			s = QString((const char*) argv[++i]);
			//app->newDocument();
			//app->doc()->loadWorkspaceAs(s);
			result = true;
		}
		else
		{
			result = true;
		}
	}

	return result;
}


/**
 * THE entry point for the application
 *
 * @param argc Number of arguments in array argv
 * @param argv Arguments array
 */
int main(int argc, char** argv)
{
	QApplication qapp(argc, argv);

	QTranslator qtTran;
	qtTran.load("qt_" + QLocale::system().name());
	qapp.installTranslator(&qtTran);

	QTranslator smTran;
	smTran.load("qlc_" + QLocale::system().name());
	qapp.installTranslator(&smTran);

	/* Registering needed to pass signals with these types to between
	   different contexts (threads) */
	qRegisterMetaType <t_function_id>("t_function_id");
	qRegisterMetaType < QHash<t_channel,t_value> >(
						"QHash<t_channel,t_value>");

	App app;
	if (parseArgs(&app, argc, argv) == false)
		return -1;
	app.show();
	qapp.exec();

#if !defined(WIN32) && !defined(__APPLE__)
	/* Set key repeat on in case QLC is set to turn it off in operate mode.
	   It's safe to assume that most users have it always turned on. */
	Display* display;
	display = XOpenDisplay(NULL);
	Q_ASSERT(display != NULL);
	XAutoRepeatOn(display);
	XCloseDisplay(display);
#endif
	return 0;
}
