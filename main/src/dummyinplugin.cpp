/*
  Q Light Controller
  dummyinplugin.cpp

  Copyright (c) Heikki Junnila

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

#include <qstring.h>
#include <qmessagebox.h>
#include <qapplication.h>

#include "dummyinplugin.h"

DummyInPlugin::DummyInPlugin() : InputPlugin()
{
	m_version = 0x00010000;
	m_name = QString("Dummy Input");
	m_type = InputType;
}

DummyInPlugin::~DummyInPlugin()
{
}

int DummyInPlugin::open()
{
	return 0;
}

int DummyInPlugin::close()
{
	return 0;
}

t_input DummyInPlugin::inputs()
{
	return 0;
}

t_input_channel DummyInPlugin::channels(t_input input)
{
	return 0;
}

int DummyInPlugin::configure(QWidget* parentWidget)
{
	QMessageBox::information(parentWidget,
				 "Dummy Output Configuration",
				 "This plugin has no configurable options");
	return 0;
}

QString DummyInPlugin::infoText()
{
	QString str;
	QString t;

	str += QString("<HTML>");
	str += QString("<HEAD>");
	str += QString("<TITLE>Plugin Info</TITLE>");
	str += QString("</HEAD>");
	str += QString("<BODY>");

	/* Title */
	str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	str += QString("<TR>");
	str += QString("<TD BGCOLOR=\"");
	str += QApplication::palette().active().highlight().name();
	str += QString("\">");
	str += QString("<FONT COLOR=\"");
	str += QApplication::palette().active().highlightedText().name();
	str += QString("\" SIZE=\"5\">");
	str += name();
	str += QString("</FONT>");
	str += QString("</TD>");
	str += QString("</TR>");
	str += QString("</TABLE>");

	/* Version */
	str += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
	str += QString("<TR>");
	str += QString("<TD><B>Version</B></TD>");
	t.sprintf("%d.%d.%d", (version() >> 16) & 0xff,
		  (version() >> 8) & 0xff, version() & 0xff);
	str += QString("<TD>" + t +"</TD>");
	str += QString("</TR>");

	str += QString("<TR>");
	str += QString("<TD><B>Inputs</B></TD>");
	t.sprintf("%d", inputs());
	str += QString("<TD><I>" + t + "</I></TD>");
	str += QString("</TR>");
	str += QString("</TABLE>");

	/* Note */
	str += QString("<H3>NOTE</H3>");
	str += QString("<P>This plugin does absolutely nothing; ");
	str += QString("you can use this if you don't have ");
	str += QString("the necessary hardware for real control.</P>");

	str += QString("</BODY>");
	str += QString("</HTML>");

	return str;
}
