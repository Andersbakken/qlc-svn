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

extern App* _app;

extern QApplication* _qapp;

SettingsUI::SettingsUI(QWidget* parent, const char* name)
  : UI_Settings(parent, name)
{
  QString str;

  m_picturesEdit->setText(_app->settings()->pixmapPath());
  m_deviceClassesEdit->setText(_app->settings()->deviceClassPath());

  m_widgetStyleCombo->insertItem("Motif");
  m_widgetStyleCombo->insertItem("Windows");
  m_widgetStyleCombo->insertItem("Platinum");
  m_widgetStyleCombo->insertItem("CDE");
  m_widgetStyleCombo->insertItem("Motif Plus");
  m_widgetStyleCombo->insertItem("SGI");
  m_widgetStyleCombo->insertItem("Interlace");
  m_widgetStyleCombo->insertItem("System default");
  m_widgetStyleCombo->setCurrentItem(_app->settings()->widgetStyle());

  m_generalFont = _app->settings()->generalFont();
  str.sprintf("%s (%dpt)", m_generalFont.family().latin1(), m_generalFont.pointSize());
  m_generalFontEdit->setFont(m_generalFont);
  m_generalFontEdit->setText(str);

  m_smallFont = _app->settings()->smallFont();
  str.sprintf("%s (%dpt)", m_smallFont.family().latin1(), m_smallFont.pointSize());
  m_smallFontEdit->setFont(m_smallFont);
  m_smallFontEdit->setText(str);

  m_openDeviceManagerCheckBox->setChecked(_app->settings()->openDeviceManager());
  m_openLastWorkspaceCheckBox->setChecked(_app->settings()->openLastWorkspace());
}

SettingsUI::~SettingsUI()
{
}

void SettingsUI::slotPicturesBrowseClicked()
{
  QString dir;
  dir = QFileDialog::getExistingDirectory(m_picturesEdit->text(), this);

  if (dir.isEmpty() == false)
    {
      m_picturesEdit->setText(dir);
    }
}

void SettingsUI::slotDeviceClassesBrowseClicked()
{
  QString dir;
  dir = QFileDialog::getExistingDirectory(m_deviceClassesEdit->text(), this);

  if (dir.isEmpty() == false)
    {
      m_deviceClassesEdit->setText(dir);
    }
}

void SettingsUI::slotGeneralFontSelectClicked()
{
  bool ok;
  QFont font;

  font = QFontDialog::getFont(&ok, m_generalFont, this);

  if (ok)
    {
      QString str;
      str.sprintf("%s (%dpt)", font.family().latin1(), font.pointSize());
      m_generalFontEdit->setFont(font);
      m_generalFontEdit->setText(str);
      m_generalFont = font;
    }
}

void SettingsUI::slotSmallFontSelectClicked()
{
  bool ok;
  QFont font;

  font = QFontDialog::getFont(&ok, m_smallFont, this);

  if (ok)
    {
      QString str;
      str.sprintf("%s (%dpt)", font.family().latin1(), font.pointSize()); 
      m_smallFontEdit->setFont(font);
      m_smallFontEdit->setText(str);
      m_smallFont = font;
    }
}

void SettingsUI::slotWidgetStyleActivated(int s)
{
}

void SettingsUI::slotOKClicked()
{
  _app->settings()->setPixmapPath(m_picturesEdit->text());
  _app->settings()->setDeviceClassPath(m_deviceClassesEdit->text());

  _app->settings()->setGeneralFont(m_generalFont);
  _app->settings()->setSmallFont(m_smallFont);

  _app->settings()->setWidgetStyle(m_widgetStyleCombo->currentItem());

  _app->settings()->setOpenDeviceManager(m_openDeviceManagerCheckBox->isChecked());
  _app->settings()->setOpenLastWorkspace(m_openLastWorkspaceCheckBox->isChecked());

  accept();
}

void SettingsUI::slotCancelClicked()
{
  reject();
}

