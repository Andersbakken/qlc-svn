/*
  Q Light Controller - Fixture Definition Editor
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
#include <QLocale>
#include <QString>

#include "app.h"

App* _app;

int main(int argc, char** argv)
{
	QApplication qapp(argc, argv);

	QTranslator qtTran;
	qtTran.load("qt_" + QLocale::system().name());
	qapp.installTranslator(&qtTran);

	QTranslator smTran;
	smTran.load("qlc_" + QLocale::system().name());
	qapp.installTranslator(&smTran);

	_app = new App(NULL);
	_app->show();

	qapp.exec();

	delete _app;

	return 0;
}
