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
#include "settings.h"

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
  m_picturesEdit->setText(_app->settings()->pixmapPath());
  m_deviceClassesEdit->setText(_app->settings()->deviceClassPath());
  m_pluginsEdit->setText(_app->settings()->pluginPath());

  fillStyleCombo();

  m_openDeviceManagerCheckBox->setChecked(_app->settings()->openDeviceManager());
  m_openLastWorkspaceCheckBox->setChecked(_app->settings()->openLastWorkspace());
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

SettingsUI::~SettingsUI()
{
}

void SettingsUI::slotSystemBrowseClicked()
{
  QString dir;
  dir = QFileDialog::getExistingDirectory(m_systemEdit->text(), this);

  if (dir.isEmpty() == false)
    {
      m_systemEdit->setText(dir);
      m_picturesEdit->setText(dir + _app->settings()->pixmapPathRelative());
      m_deviceClassesEdit->setText(dir + _app->settings()->deviceClassPathRelative());
      m_pluginsEdit->setText(dir + _app->settings()->pluginPathRelative());
    }
}

void SettingsUI::slotSystemEditTextChanged(const QString &text)
{
  QString path(text);

  if (path.right(1) != QString("/"))
    {
      path += QString("/");
    }

  m_picturesEdit->setText(path + _app->settings()->pixmapPathRelative());
  m_deviceClassesEdit->setText(path + _app->settings()->deviceClassPathRelative());
  m_pluginsEdit->setText(path + _app->settings()->pluginPathRelative());
}

void SettingsUI::slotWidgetStyleActivated(int s)
{
}

void SettingsUI::slotOKClicked()
{
  _app->settings()->setSystemPath(m_systemEdit->text());

  _app->settings()->setWidgetStyle(m_widgetStyleCombo->currentText());
  
  _app->settings()->setOpenDeviceManager(m_openDeviceManagerCheckBox->isChecked());
  _app->settings()->setOpenLastWorkspace(m_openLastWorkspaceCheckBox->isChecked());

  accept();
}

void SettingsUI::slotStyleChanged(const QString &)
{
  _app->settings()->setWidgetStyle(m_widgetStyleCombo->currentText());
  _qapp->setStyle( m_widgetStyleCombo->currentText());
}

void SettingsUI::slotCancelClicked()
{
  reject();
}

