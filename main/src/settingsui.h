/*
  Q Light Controller
  settingsui.h
  
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

#ifndef SETTINGSUI_H
#define SETTINGSUI_H

#include "uic_settings.h"

class SettingsUI : public UI_Settings
{
  Q_OBJECT

 public:
  SettingsUI(QWidget* parent);
  ~SettingsUI();

  void init();

 private slots:
  void slotBackgroundBrowseClicked();
  void slotStyleChanged(const QString &);
  void slotConfigurePluginClicked();
  void slotOKClicked();
  void slotCancelClicked();

 private:
  void fillStyleCombo();
  void fillOutputPluginCombo();
  void fillAdvancedSettingsList();
};

#endif
