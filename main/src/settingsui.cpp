/*
  Q Light Controller
  settingsui.cpp

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

#include "settingsui.h"
#include "app.h"
#include "doc.h"
#include "settings.h"

#include "../../libs/common/plugin.h"

#include <qfontdialog.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qcheckbox.h>
#include <qstylefactory.h>

extern App* _app;

extern QApplication* _qapp;

SettingsUI::SettingsUI(QWidget* parent, const char* name)
  : UI_Settings(parent, name)
{
  QString str;

  m_systemEdit->setText(_app->settings()->systemPath());

  fillStyleCombo();
  fillOutputPluginCombo();

  m_openDeviceManagerCheckBox->setChecked(_app->settings()->openDeviceManager());
  m_openLastWorkspaceCheckBox->setChecked(_app->settings()->openLastWorkspace());
  m_keyRepeatCheckBox->setChecked(_app->settings()->keyRepeatOffInOperateMode());
}

SettingsUI::~SettingsUI()
{
}

void SettingsUI::fillStyleCombo()
{
  QStyleFactory f;
  QStringList l = f.keys();
  
  for (QStringList::Iterator it = l.begin(); it != l.end(); ++it)
    {
      m_widgetStyleCombo->insertItem(*it);
    }
  
  for (int i = 0; i < m_widgetStyleCombo->count(); i++)
    {
      if (m_widgetStyleCombo->text(i) == _app->settings()->widgetStyle())
	{
	  m_widgetStyleCombo->setCurrentItem(i);
	  break;
	}
    }
}

void SettingsUI::fillOutputPluginCombo()
{
  QList <Plugin> *pl = _app->doc()->pluginList();

  m_outputPluginCombo->clear();

  for (unsigned int i = 0; i < pl->count(); i++)
    {
      Plugin* plugin = pl->at(i);
      ASSERT(plugin != NULL);

      if (plugin->type() == Plugin::OutputType)
	{
	  m_outputPluginCombo->insertItem(plugin->name(), 0);
	}
    }

  for (int i = 0; i < m_outputPluginCombo->count(); i++)
    {
      if (m_outputPluginCombo->text(i) == _app->settings()->outputPlugin())
	{
	  m_outputPluginCombo->setCurrentItem(i);
	  break;
	}
    }
}

void SettingsUI::slotSystemBrowseClicked()
{
  QString dir;
  dir = QFileDialog::getExistingDirectory(m_systemEdit->text(), this);

  if (dir.isEmpty() == false)
    {
      m_systemEdit->setText(dir);
    }
}

void SettingsUI::slotStyleChanged(const QString &)
{
  _app->settings()->setWidgetStyle(m_widgetStyleCombo->currentText());
  _qapp->setStyle( m_widgetStyleCombo->currentText());
}

void SettingsUI::slotOKClicked()
{
  _app->settings()->setSystemPath(m_systemEdit->text());

  _app->settings()->setWidgetStyle(m_widgetStyleCombo->currentText());
  
  _app->settings()->setOpenDeviceManager(m_openDeviceManagerCheckBox->isChecked());
  _app->settings()->setOpenLastWorkspace(m_openLastWorkspaceCheckBox->isChecked());

  _app->settings()->setKeyRepeatOffInOperateMode(m_keyRepeatCheckBox->isChecked());

  _app->settings()->setOutputPlugin(m_outputPluginCombo->currentText());

  accept();
}

void SettingsUI::slotCancelClicked()
{
  reject();
}

