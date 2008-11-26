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

#include <unistd.h>

Configure_uDMXOut::Configure_uDMXOut(QWidget* parent, uDMXOut* plugin)
	: QDialog(parent)
{
  m_plugin = plugin ;

  //m_plugin->Debug(DEBUG_ALL, "%s %d %s()", __FILE__, __LINE__, __FUNCTION__) ;  
  
  setupUi(this);

  m_firstUnivSpinBox->setValue(m_plugin->firstUniverse());
  m_spnChannel->setValue(m_plugin->Channels()) ;
  m_cmbDebug->setCurrentIndex(m_plugin->Debug()) ;

  updateStatus();
}

Configure_uDMXOut::~Configure_uDMXOut()
{
  //m_plugin->Debug(DEBUG_ALL, "%s %d %s()", __FILE__, __LINE__, __FUNCTION__) ; 
}

QString Configure_uDMXOut::device()
{
  //m_plugin->Debug(DEBUG_ALL, "%s %d %s()", __FILE__, __LINE__, __FUNCTION__) ; 
  
  return "uDMX";
}

int Configure_uDMXOut::firstUniverse()
{
  //m_plugin->Debug(DEBUG_ALL, "%s %d %s()", __FILE__, __LINE__, __FUNCTION__) ; 

  return m_firstUnivSpinBox->value();
}

int Configure_uDMXOut::Channels()
{
  return m_spnChannel->value();
}

int Configure_uDMXOut::Debug()
{
  return m_cmbDebug->currentIndex() ;
  //return 0 ;
}

void Configure_uDMXOut::slotActivateClicked()
{
  m_plugin->activate();

  //::usleep(10);  // Allow the activation signal get passed to doc

  updateStatus();
}

void Configure_uDMXOut::updateStatus()
{
  if (m_plugin->isOpen())
  {
    m_statusLabel->setText("Active");
    m_activate->setEnabled(false);
  }
  else
  {
    m_statusLabel->setText("Not Active");
    m_activate->setEnabled(true);
  }
}
