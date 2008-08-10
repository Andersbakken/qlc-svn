/*
  Q Light Controller
  aboutbox.cpp

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

#include <QListWidget>
#include <QPushButton>
#include <QPixmap>
#include <QLabel>
#include <QColor>
#include <QFont>

#include "aboutbox.h"
#include "app.h"

AboutBox::AboutBox(QWidget* parent) : QDialog (parent)
{
	initDialog();
}

AboutBox::~AboutBox()
{
	delete m_pm;
}

void AboutBox::initDialog()
{
	int w = 350;
	int h = 123;
	QPalette pal;
	QFont font;

	setWindowTitle(QString("About ") + KApplicationNameLong);
	setFixedSize(w, h + 220);

	/* Background color */
	pal = palette();
	pal.setColor(QPalette::Window, Qt::white);
	setPalette(pal);

	m_pm = new QPixmap(":/qlc-big.png");
	m_logo = new QLabel(this);

	pal = m_logo->palette();
	pal.setColor(QPalette::Window, Qt::white);
	m_logo->setPalette(pal);

	if (m_pm->isNull() == false)
	{
		m_logo->setGeometry(50, 0, 254, h);
		m_logo->setPixmap(*m_pm);
	}
	else
	{
		m_logo->setGeometry(0, 0, w, h);
		m_logo->setAlignment(Qt::AlignCenter);
		m_logo->setText("Incorrect pixmap path!");
	}

	font.setPixelSize(12);

	m_version = new QLabel(this);
	m_version->setFont(font);
	m_version->setGeometry(0, h, w, 20);
	m_version->setAlignment(Qt::AlignCenter);
	m_version->setText(KApplicationNameLong + " " + KApplicationVersion);
	pal = m_version->palette();
	pal.setColor(QPalette::Window, Qt::white);
	m_version->setPalette(pal);

	m_copyright = new QLabel(this);
	m_copyright->setFont(font);
	m_copyright->setGeometry(0, h + 30, w, 50);
	m_copyright->setAlignment(Qt::AlignCenter);
	m_copyright->setTextFormat(Qt::RichText);
	m_copyright->setText("QLC is licensed under the GNU GPL and is<BR>copyrighted &copy; by<BR><B>the people involved in QLC development:</B>");
	pal = m_copyright->palette();
	pal.setColor(QPalette::Window, Qt::white);
	m_copyright->setPalette(pal);

	m_people = new QListWidget(this);
	m_people->setFont(font);
	m_people->setGeometry(5, h + 90, w - 10, 80);
	m_people->setFocusPolicy(Qt::NoFocus);
	m_people->setSelectionMode(QAbstractItemView::NoSelection);
	m_people->setFrameStyle(QFrame::Panel);
	m_people->addItem("Heikki Junnila <hjunnila@iki.fi>");
	m_people->addItem("Stefan Krumm <krumm@geol.uni-erlangen.de>");
	m_people->addItem("Christian Sühs <dance-or-die@users.sf.net>");
	m_people->addItem("Christopher Staite <chris@yourdreamnet.co.uk>");
	pal = m_people->palette();
	pal.setColor(QPalette::Window, Qt::white);
	m_people->setPalette(pal);

	m_ok = new QPushButton(this);
	m_ok->setGeometry(133, h + 180, 100, 30);
	m_ok->setMinimumSize(100, 30);
	m_ok->setDefault(false);
	m_ok->setText("&OK");
	pal = m_ok->palette();
	pal.setColor(QPalette::Window, Qt::white);
	m_ok->setPalette(pal);

	connect(m_ok, SIGNAL(clicked()), this, SLOT(accept()));
}
