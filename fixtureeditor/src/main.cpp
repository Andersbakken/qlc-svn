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

#include "app.h"

App* _app;
QApplication* _qapp;

void print_version()
{
	qDebug(" ");
	qDebug(KApplicationNameLong + " " + KApplicationVersion);
	qDebug("This program is licensed under the terms of the GNU GPL.");
	qDebug("Copyright (c) Heikki Junnila (hjunnila@users.sf.net)");
	qDebug(" ");
}

void print_usage()
{
	print_version();

	qDebug("Usage:");
	qDebug("  dce [options]");
        qDebug("\nOptions:");
        qDebug("  -h or --help                  Print this help");
        qDebug("  -v or --version               Print version information");
	qDebug(" ");
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
			if (::strcmp(argv[i], "-v") == 0 ||
			    ::strcmp(argv[i], "--version") == 0)
			{
				print_version();
				ret = 1;
			}
			else if (::strcmp(argv[i], "-h") == 0 ||
				 ::strcmp(argv[i], "--help") == 0)
			{
				print_usage();
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

	//
	// Parse any command line arguments
	if (parseArgs(argc, argv) != 0)
	{
		return 0;
	}

	//
	// Initialize QApplication object
	_qapp = new QApplication(argc, argv);

	//
	// Construct the main application class
	_app = new App();
	_qapp->setMainWidget(_app);
	_app->setCaption(KApplicationNameLong);
	_app->initView();
	_app->show();

	//
	// Main application loop
	result = _qapp->exec();

	//
	// Delete main app class
	delete _app;
	delete _qapp;

	return result;
}
