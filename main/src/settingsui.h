/*
  Q Light Controller
  settingsui.h
  
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

#ifndef SETTINGSUI_H
#define SETTINGSUI_H

#include "classes.h"
#include "uic_settings.h"

class QFont;

class SettingsUI : public UI_Settings
{
  Q_OBJECT

  friend class Settings;

 public:
  SettingsUI(QWidget* parent = NULL, const char* name = NULL);
  ~SettingsUI();

 public slots:
  void slotSystemBrowseClicked();
  void slotSystemEditTextChanged(const QString &);

  void slotGeneralFontSelectClicked();
  void slotSmallFontSelectClicked();
  void slotOKClicked();
  void slotCancelClicked();
  void slotWidgetStyleActivated(int);

 private:
  QString m_picturePath;
  QString m_deviceClassPath;

  QFont m_generalFont;
  QFont m_smallFont;

  bool m_openDeviceManager;
  bool m_openLastWorkspace;

};

#endif
