/*
  Q Light Controller
  configureolaout.cpp

  Copyright (C) Simon Newton,
                Heikki Junnila

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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QPushButton>
#include <QDialog>
#include <QString>
#include <QTimer>

#include "configureolaout.h"
#include "olaout.h"

static const unsigned int KColumnName = 0;
static const unsigned int KColumnOutput = 1;


/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureOLAOut::ConfigureOLAOut(QWidget* parent, OLAOut* plugin)
    : QDialog(parent)
{
  Q_ASSERT(plugin != NULL);
  m_plugin = plugin;

  setupUi(this);
  populateOutputList();
	
  m_standaloneCheck->setChecked(m_plugin->isServerEmbedded());
}

ConfigureOLAOut::~ConfigureOLAOut()
{
  m_plugin->setServerEmbedded(m_standaloneCheck->isChecked());
}

/*****************************************************************************
 * Refresh
 *****************************************************************************/

void ConfigureOLAOut::populateOutputList()
{
  m_listView->clear();
  OutputList outputs = m_plugin->outputMapping();

  for (int i = 0; i != outputs.size(); ++i)
  {
    QTreeWidgetItem* item = new QTreeWidgetItem(m_listView);
    item->setText(KColumnName, QString("OLA Output %1").arg(i + 1));
    item->setText(KColumnOutput, QString("%1").arg(outputs[i]));
  }
}
