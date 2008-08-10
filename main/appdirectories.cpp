/*
  Q Light Controller
  appdirectories.cpp
  
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

#include <QFileDialog>
#include <QToolButton>
#include <QLineEdit>
#include <QSettings>
#include <QIcon>

#include "appdirectories.h"

AppDirectories::AppDirectories(QWidget* parent) : QDialog(parent)
{
	QSettings set;
	QString str;
	
	setupUi(this);
	
	/* Background image */
	m_bgButton->setIcon(QIcon(":/fileopen.png"));
	str = set.value("workspace/background").toString();
	m_bgEdit->setText(str);
	m_bgGroup->setChecked(!str.isEmpty());
	connect(m_bgButton, SIGNAL(clicked()), this, SLOT(slotBGClicked()));

	/* Fixtures */
	m_fixturesButton->setIcon(QIcon(":/fileopen.png"));
	str = set.value("directories/fixtures").toString();
	m_fixturesEdit->setText(str);
	connect(m_fixturesButton, SIGNAL(clicked()),
		this, SLOT(slotFixturesClicked()));
	
	/* Plugins */
	m_pluginsButton->setIcon(QIcon(":/fileopen.png"));
	str = set.value("directories/plugins").toString();
	m_pluginsEdit->setText(str);
	connect(m_pluginsButton, SIGNAL(clicked()),
		this, SLOT(slotPluginsClicked()));
	
	/* Documentation */
	m_docsButton->setIcon(QIcon(":/fileopen.png"));
	str = set.value("directories/documentation").toString();
	m_docsEdit->setText(str);
	connect(m_docsButton, SIGNAL(clicked()), this, SLOT(slotDocsClicked()));
}

AppDirectories::~AppDirectories()
{
}

/****************************************************************************
 * Slots
 ****************************************************************************/
 
void AppDirectories::slotBGClicked()
{
	QString path;
	path = QFileDialog::getOpenFileName(this, tr("Open an image"),
					m_bgEdit->text(),
					tr("Image files (*.png *.jpg *.bmp)"));
	if (path.isEmpty() == false)
		m_bgEdit->setText(path);
}

void AppDirectories::slotFixturesClicked()
{
	QString path;
	path = QFileDialog::getExistingDirectory(this,
						 tr("Fixture directory"),
						 m_bgEdit->text(),
						 QFileDialog::ShowDirsOnly);
	if (path.isEmpty() == false)
		m_fixturesEdit->setText(path);
}

void AppDirectories::slotPluginsClicked()
{
	QString path;
	path = QFileDialog::getExistingDirectory(this,
						 tr("Plugin directory"),
						 m_bgEdit->text(),
						 QFileDialog::ShowDirsOnly);
	if (path.isEmpty() == false)
		m_pluginsEdit->setText(path);
}

void AppDirectories::slotDocsClicked()
{
	QString path;
	path = QFileDialog::getExistingDirectory(this,
						 tr("Documentation directory"),
						 m_bgEdit->text(),
						 QFileDialog::ShowDirsOnly);
	if (path.isEmpty() == false)
		m_docsEdit->setText(path);
}

void AppDirectories::accept()
{
	QSettings set;

	set.setValue("workspace/background", m_bgEdit->text());
	set.setValue("directories/fixtures", m_fixturesEdit->text());
	set.setValue("directories/plugins", m_pluginsEdit->text());
	set.setValue("directories/documentation", m_docsEdit->text());
	
	QDialog::accept();
}
