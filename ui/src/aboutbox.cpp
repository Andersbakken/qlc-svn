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

#include <QLabel>

#include "aboutbox.h"
#include "app.h"

AboutBox::AboutBox(QWidget* parent) : QDialog (parent)
{
	setupUi(this);

	m_titleLabel->setText(App::longName());
	m_versionLabel->setText(App::version());
	m_copyrightLabel->setText(QString("Copyright &copy; Heikki Junnila %1")
				.arg(tr("and contributors:")));
	m_websiteLabel->setText(tr("Website: %1").arg("<A HREF=\"http://www.sf.net/projects/qlc\">http://www.sf.net/projects/qlc</a>"));

	m_contributors->clear();
	m_contributors->addItem("Klaus Weidenbach");
	m_contributors->addItem("Stefan Krumm");
	m_contributors->addItem(QByteArray::fromPercentEncoding("Christian S%fchs"));
	m_contributors->addItem("Simon Newton");
	m_contributors->addItem("Christopher Staite");
	m_contributors->addItem("Lutz Hillebrand");
}

AboutBox::~AboutBox()
{
}

