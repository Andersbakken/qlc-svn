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
#include "configkeys.h"

#include "../../libs/common/plugin.h"

#include <qfontdialog.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qcheckbox.h>
#include <qstylefactory.h>
#include <qpixmap.h>
#include <qworkspace.h>

extern App* _app;

extern QApplication* _qapp;

SettingsUI::SettingsUI(QWidget* parent, const char* name)
  : UI_Settings(parent, name)
{
  QString str;

  _app->settings()->get(KEY_SYSTEM_DIR, str);
  m_systemEdit->setText(str);

  _app->settings()->get(KEY_APP_BACKGROUND, str);
  m_backgroundEdit->setText(str);

  fillStyleCombo();
  fillOutputPluginCombo();

  _app->settings()->get(KEY_DEVICE_MANAGER_OPEN, str);
  m_openDeviceManagerCheckBox->setChecked((str == Settings::trueValue()) ? true : false);

  _app->settings()->get(KEY_OPEN_LAST_WORKSPACE, str);
  m_openLastWorkspaceCheckBox->setChecked((str == Settings::trueValue()) ? true : false);

  _app->settings()->get(KEY_KEY_REPEAT, str);
  m_keyRepeatCheckBox->setChecked((str == Settings::trueValue()) ? true : false);
}

SettingsUI::~SettingsUI()
{
}

void SettingsUI::fillStyleCombo()
{
  QStyleFactory f;
  QStringList l = f.keys();

  QString widgetStyle;
  _app->settings()->get(KEY_WIDGET_STYLE, widgetStyle);
  
  for (QStringList::Iterator it = l.begin(); it != l.end(); ++it)
    {
      m_widgetStyleCombo->insertItem(*it);
    }
  
  for (int i = 0; i < m_widgetStyleCombo->count(); i++)
    {
      if (m_widgetStyleCombo->text(i) == widgetStyle)
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

  QString outputPlugin;
  _app->settings()->get(KEY_OUTPUT_PLUGIN, outputPlugin);

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
      if (m_outputPluginCombo->text(i) == outputPlugin)
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

void SettingsUI::slotBackgroundBrowseClicked()
{
  QString path;
  path = QFileDialog::getOpenFileName(m_backgroundEdit->text(), QString("Images (*.png *.xpm *.jpg *.gif)"), this);

  if (path.isEmpty() == false)
    {
      m_backgroundEdit->setText(path);
      _app->workspace()->setBackgroundPixmap(QPixmap(path));
    }
}

void SettingsUI::slotStyleChanged(const QString &)
{
  _app->settings()->set(KEY_WIDGET_STYLE, m_widgetStyleCombo->currentText());
  _qapp->setStyle(m_widgetStyleCombo->currentText());
}

void SettingsUI::slotOKClicked()
{
  _app->settings()->set(KEY_SYSTEM_DIR, m_systemEdit->text());
  _app->settings()->set(KEY_APP_BACKGROUND, m_backgroundEdit->text());

  _app->settings()->set(KEY_WIDGET_STYLE, m_widgetStyleCombo->currentText());
  

  if (m_openDeviceManagerCheckBox->isChecked())
    {
      _app->settings()->set(KEY_DEVICE_MANAGER_OPEN, Settings::trueValue());
    }
  else
    {
      _app->settings()->set(KEY_DEVICE_MANAGER_OPEN, Settings::falseValue());
    }

  if (m_openLastWorkspaceCheckBox->isChecked())
    {
      _app->settings()->set(KEY_OPEN_LAST_WORKSPACE, Settings::trueValue());
    }
  else
    {
      _app->settings()->set(KEY_OPEN_LAST_WORKSPACE, Settings::falseValue());
    }

  if (m_keyRepeatCheckBox->isChecked())
    {
      _app->settings()->set(KEY_KEY_REPEAT, Settings::trueValue());
    }
  else
    {
      _app->settings()->set(KEY_KEY_REPEAT, Settings::falseValue());
    }

  _app->settings()->set(KEY_OUTPUT_PLUGIN, m_outputPluginCombo->currentText());

  accept();
}

void SettingsUI::slotCancelClicked()
{
  reject();
}

