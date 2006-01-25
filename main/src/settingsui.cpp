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
#include "common/settings.h"
#include "common/configitem.h"
#include "configkeys.h"
#include "imagecontentspreview.h"
#include "dummyoutplugin.h"

#include <qlistview.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qcheckbox.h>
#include <qstylefactory.h>
#include <qpixmap.h>
#include <qworkspace.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qpushbutton.h>

extern App* _app;

extern QApplication* _qapp;

SettingsUI::SettingsUI(QWidget* parent) : UI_Settings(parent)
{
}

SettingsUI::~SettingsUI()
{
}

void SettingsUI::init()
{
  QString str;

  m_MRUSizeLabel->setEnabled(false);
  m_MRUSizeSpin->setEnabled(false);

  if (_app->settings()->get(KEY_APP_BACKGROUND, str) != -1)
  {
	m_backgroundEdit->setText(str);
  }

  m_backgroundBrowse->setPixmap(QPixmap(QString(PIXMAPS) +
					QString("/fileopen.png")));

  fillStyleCombo();

  if (_app->settings()->get(KEY_OPEN_LAST_WORKSPACE, str) != -1
	&& str == Settings::trueValue())
  {
	m_openLastWorkspaceCheckBox->setChecked(true);
  }
  else
  {
	m_openLastWorkspaceCheckBox->setChecked(false);
  }

  fillOutputPluginCombo();

  m_configurePlugin->setPixmap(QPixmap(QString(PIXMAPS) +
				QString("/configure.png")));

  fillAdvancedSettingsList();
}

void SettingsUI::fillStyleCombo()
{
  QStyleFactory f;
  QStringList l = f.keys();

  QString widgetStyle;
  if (_app->settings()->get(KEY_WIDGET_STYLE, widgetStyle) == -1 ||
	widgetStyle == "")
  {
	widgetStyle = "Default";
  }

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
  QPtrListIterator <Plugin> it(*_app->pluginList());
  int i = 0;

  QString plugin;
  if (_app->settings()->get(KEY_OUTPUT_PLUGIN, plugin) == -1 ||
	plugin == "")
  {
	  plugin = DummyOutPlugin::PluginName;
  }

  while (it.current() != NULL)
    {
      if (it.current()->type() == Plugin::OutputType)
	{
	  m_outputPluginCombo->insertItem(it.current()->name(), i);
	  if (it.current()->name() == plugin)
	    {
	      m_outputPluginCombo->setCurrentItem(i);
	    }

	  i++;
	}

      ++it;
    }
}


void SettingsUI::slotConfigurePluginClicked()
{
  OutputPlugin* p = static_cast<OutputPlugin*>
    (_app->searchPlugin(m_outputPluginCombo->currentText(),
			Plugin::OutputType));
  ASSERT(p);

  p->configure();
}

void SettingsUI::fillAdvancedSettingsList()
{
  QListViewItem* item = NULL;
  QPtrList <ConfigItem>* items = _app->settings()->items();

  m_advancedList->clear();

  for (ConfigItem* i = items->first(); i != NULL; i = items->next())
    {
      item = new QListViewItem(m_advancedList, *(i->key()), *(i->text()));
      item->setRenameEnabled(1, false);
    }
}

void SettingsUI::slotBackgroundBrowseClicked()
{
  QString path;

  ImageContentsPreview* p = new ImageContentsPreview;

  QFileDialog* fd = new QFileDialog( this );
  fd->setCaption("Choose the workspace background image");
  fd->setContentsPreviewEnabled( TRUE );
  fd->setContentsPreview( p, p );
  fd->setPreviewMode( QFileDialog::Contents );
  fd->setFilter("Images (*.png *.xpm *.jpg *.gif)");
  fd->setSelection(m_backgroundEdit->text());

  if (fd->exec() == QDialog::Accepted)
    {
      path = fd->selectedFile();

      if (path.isEmpty() == false)
	{
	  m_backgroundEdit->setText(path);
	}
    }

  delete p;
  delete fd;
}


void SettingsUI::slotStyleChanged(const QString &)
{
  _app->settings()->set(KEY_WIDGET_STYLE, m_widgetStyleCombo->currentText());
  _qapp->setStyle(m_widgetStyleCombo->currentText());
}


void SettingsUI::slotOKClicked()
{
  _app->settings()->set(KEY_APP_BACKGROUND, m_backgroundEdit->text());
  _app->settings()->set(KEY_WIDGET_STYLE, m_widgetStyleCombo->currentText());
  _app->settings()->set(KEY_OUTPUT_PLUGIN, m_outputPluginCombo->currentText());

  if (m_openLastWorkspaceCheckBox->isChecked())
    {
      _app->settings()->set(KEY_OPEN_LAST_WORKSPACE, Settings::trueValue());
    }
  else
    {
      _app->settings()->set(KEY_OPEN_LAST_WORKSPACE, Settings::falseValue());
    }

  _app->workspace()->setBackgroundPixmap(QPixmap(m_backgroundEdit->text()));

  accept();
}

void SettingsUI::slotCancelClicked()
{
  reject();
}
