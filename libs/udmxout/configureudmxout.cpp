/*
  Q Light Controller
  configureuDMXout.cpp

  Copyright (c)	2008, Lutz Hillebrand (ilLUTZminator)

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

#include "configureudmxout.h"
#include "udmxout.h"

//  Insert used widget-includes ...
#include <QString>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QMessageBox>

Configure_uDMXOut::Configure_uDMXOut(QWidget* parent, uDMXOut* plugin)
	: QDialog(parent)
{
	m_plugin = plugin;

	// m_plugin->Debug(DEBUG_ALL, "%s %d %s()", __FILE__, __LINE__, __FUNCTION__);  
  
	setupUi(this);

	m_firstUniverseSpin->setValue(m_plugin->firstUniverse());
	m_channelsSpin->setValue(m_plugin->channels());
	m_debugCombo->setCurrentIndex(m_plugin->debug());

	/* TODO: Make some use of these controls or remove them */
	m_firstUniverseSpin->hide();
	m_firstUniverseLabel->hide();
	m_debugCombo->hide();
	m_debugLabel->hide();
}

Configure_uDMXOut::~Configure_uDMXOut()
{
	// m_plugin->Debug(DEBUG_ALL, "%s %d %s()", __FILE__, __LINE__, __FUNCTION__); 
}

int Configure_uDMXOut::firstUniverse()
{
	// m_plugin->Debug(DEBUG_ALL, "%s %d %s()", __FILE__, __LINE__, __FUNCTION__);
	return m_firstUniverseSpin->value();
}

int Configure_uDMXOut::channels()
{
	return m_channelsSpin->value();
}

int Configure_uDMXOut::debug()
{
	return m_debugCombo->currentIndex();
}

