/*
  Q Light Controller - Device Class Editor
  deviceclasseditor.cpp

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

#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qspinbox.h>

#include "app.h"
#include "deviceclasseditor.h"
#include "editpresetvalue.h"

#include "../../main/src/settings.h"
#include "../../main/src/configkeys.h"
#include "../../main/src/deviceclass.h"
#include "../../main/src/logicalchannel.h"
#include "../../main/src/capability.h"

extern App* _app;

const int KChannelNumberColumn(0);
const int KChannelNameColumn(1);

const int KCapabilityMinimumColumn(0);
const int KCapabilityMaximumColumn(1);
const int KCapabilityDescriptionColumn(2);

DeviceClassEditor::DeviceClassEditor(QWidget* parent, DeviceClass* dc)
  : UI_DeviceClassEditor(parent)
{
  m_dc = dc;
  m_modified = false;
}

DeviceClassEditor::~DeviceClassEditor()
{
  delete m_dc;
}

void DeviceClassEditor::init()
{
  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

  setIcon(QPixmap(dir + "/device.xpm"));

  setCaption(m_dc->manufacturer() + QString(" - ") + m_dc->model());

  m_manufacturerEdit->setText(m_dc->manufacturer());
  m_modelEdit->setText(m_dc->model());

  m_typeCombo->setCurrentText(m_dc->type());

  updateChannelList();
  updatePresetValues();

  setModified(false);
}

void DeviceClassEditor::closeEvent(QCloseEvent* e)
{
  if (m_modified)
    {
      int r = QMessageBox::information(this, KApplicationNameShort,
			      "Do you want to save changes to device class\n\""
			      + m_dc->name() + "\"\nbefore closing?",
			      QMessageBox::Yes, QMessageBox::No, 
				       QMessageBox::Cancel);
      if (r == QMessageBox::Yes)
	{
	  if (save())
	    {
	      e->accept();
	      emit closed(this);
	    }
	}
      else if (r == QMessageBox::No)
	{
	  e->accept();
	  emit closed(this);
	}
      else
	{
	  e->ignore();
	}
    }
  else
    {
      e->accept();
      emit closed(this);
    }
}

bool DeviceClassEditor::save()
{
  if (m_fileName == QString::null)
    {
      return saveAs();
    }
  else
    {
      m_dc->saveToFile(m_fileName);
      
      setModified(false);
      
      return true;
    }
}

bool DeviceClassEditor::saveAs()
{
  QString path;

  if (m_fileName == QString::null)
    {
      _app->settings()->get(KEY_SYSTEM_DIR, path);
      path += QString("/") + DEVICECLASSPATH + QString("/");
      path += m_dc->manufacturer() + QString("-") + 
	m_dc->model() + QString(".deviceclass");
    }
  else
    {
      path = m_fileName;
    }

  path = QFileDialog::getSaveFileName(path, "Device Classes (*.deviceclass)", 
				      this);

  if (path != QString::null)
    {
      if (path.right(12) != QString(".deviceclass"))
	{
	  path += QString(".deviceclass");
	}
      
      m_dc->saveToFile(path);
      m_fileName = QString(path);

      setModified(false);

      return true;
    }
  else
    {
      return false;
    }
}

void DeviceClassEditor::setModified(bool modified)
{
  if (modified == true)
    {
      setCaption(m_dc->manufacturer() + QString(" - ") +
		 m_dc->model() + QString(" *"));
    }
  else
    {
      setCaption(m_dc->manufacturer() + QString(" - ") + m_dc->model());
    }

  m_modified = modified;
}

void DeviceClassEditor::slotManufacturerEditTextChanged(const QString &text)
{
  m_dc->setManufacturer(text);
  setCaption(m_dc->manufacturer() + QString(" - ") + m_dc->model());

  setModified();
}

void DeviceClassEditor::slotModelEditTextChanged(const QString &text)
{
  m_dc->setModel(text);
  setCaption(m_dc->manufacturer() + QString(" - ") + m_dc->model());

  setModified();
}

void DeviceClassEditor::slotTypeSelected(const QString &text)
{
  m_dc->setType(text);
  setModified();
}

void DeviceClassEditor::slotChannelListSelectionChanged(QListViewItem* item)
{
  updatePresetValues();
}

void DeviceClassEditor::slotPresetListSelectionChanged(QListViewItem* item)
{
}

void DeviceClassEditor::slotAddChannelClicked()
{
  bool ok = false;
  QString text = QInputDialog::getText(KApplicationNameShort,
                                  QString("Enter description for new channel"),
                                  QLineEdit::Normal, QString::null,
				  &ok, this);
  if (ok == true && text.isEmpty() == false)
    {
      LogicalChannel* lc = new LogicalChannel();
      lc->setName(text);
      lc->setChannel(m_dc->channels()->count()); // 0, 1, 2...

      Capability* c = new Capability();
      c->setLo(0);
      c->setHi(255);
      c->setName(QString("Channel Level"));
      lc->capabilities()->append(c);
      
      m_dc->channels()->append(lc);

      updateChannelList();

      for (QListViewItem* item = m_channelList->firstChild(); item != NULL;
           item = item->nextSibling())
        {
          if (item->text(KChannelNameColumn) == text)
            {
              m_channelList->setSelected(item, true);
              break;
            }
        }

      setModified();
    }
}

void DeviceClassEditor::slotRemoveChannelClicked()
{
  QPtrList <LogicalChannel> *cl = m_dc->channels();

  LogicalChannel* ch = currentChannel();

  cl->remove( currentChannel() );
  delete ch;
  ch = NULL;

  // Reorganise channel numbers
  int i = 0;
  for (LogicalChannel* c = cl->first(); c != NULL; c = cl->next())
    {
      c->setChannel(i++);
    }

  updateChannelList();
  updatePresetValues();
  
  setModified();
}

void DeviceClassEditor::slotEditChannelClicked()
{
  LogicalChannel* ch = currentChannel();

  if (ch == NULL)
    {
      return;
    }

  bool ok = false;
  QString text = QInputDialog::getText(KApplicationNameShort,
                                       QString("Enter channel description"),
                                       QLineEdit::Normal,
                                       ch->name(), &ok, this);
  if (ok == true && text.isEmpty() == false)
    {
      ch->setName(text);
      updateChannelList();
      
      setModified();
    }
}

void DeviceClassEditor::slotRaiseChannelClicked()
{
  LogicalChannel* ch = currentChannel();

  if (ch == NULL)
    {
      return;
    }

  unsigned int i = m_dc->channels()->find(ch);
  if (i == 0)
    {
      return;
    }
  else
    {
      m_dc->channels()->at(i - 1)->setChannel(i);
      m_dc->channels()->take(i);
      i--;
      ch->setChannel(i);
      m_dc->channels()->insert(i, ch);
      updateChannelList();
      
      setModified();
      
      for (QListViewItem* item = m_channelList->firstChild(); item != NULL; 
	   item = item->nextSibling())
        {
          QString num;
          num.sprintf("%03d", i + 1);
          if (item->text(KChannelNumberColumn) == num)
            {
              m_channelList->setSelected(item, true);
              slotChannelListSelectionChanged(item);
              break;
            }
        }
    }
}

void DeviceClassEditor::slotLowerChannelClicked()
{
  LogicalChannel* ch = currentChannel();

  if (ch == NULL)
    {
      return;
    }

  unsigned int i = m_dc->channels()->find(ch);
  if (i == m_dc->channels()->count() - 1)
    {
      return;
    }
  else
    {
      m_dc->channels()->at(i + 1)->setChannel(i);
      m_dc->channels()->take(i);
      i++;
      ch->setChannel(i);
      m_dc->channels()->insert(i, ch);
      updateChannelList();
      
      setModified();
      
      for (QListViewItem* item = m_channelList->firstChild(); item != NULL;
	   item = item->nextSibling())
        {
          QString num;
          num.sprintf("%03d", i + 1);
          if (item->text(KChannelNumberColumn) == num)
            {
              m_channelList->setSelected(item, true);
              slotChannelListSelectionChanged(item);
              break;
            }
        }
    }
}

void DeviceClassEditor::slotAddPresetClicked()
{
  LogicalChannel* ch = currentChannel();

  if (ch == NULL)
    {
      return;
    }
  
  EditPresetValue* epv = new EditPresetValue(this);
  
  while (1)
    {
      if (epv->exec() == QDialog::Accepted)
	{
	  int min = epv->m_minSpin->value();
	  int max = epv->m_maxSpin->value();
	  QString description = epv->m_descriptionEdit->text();
	  
	  if (ch->searchCapability(min) != NULL)
	    {
	      QMessageBox::information(this, KApplicationNameShort,
				       "Overlapping value range! Unable to add value.");
	      
	      continue;
	    }
	  else if (ch->searchCapability(max) != NULL)
	    {
	      QMessageBox::information(this, KApplicationNameShort,
				       "Overlapping value range! Unable to add value.");
	      continue;
	    }
	  else
	    {
	      Capability* cap = new Capability();
	      cap->setLo(min);
	      cap->setHi(max);
	      cap->setName(description);
	      
	      ch->capabilities()->append(cap);
	      
	      setModified();
	      
	      updatePresetValues();
	      break;
	    }
	}
      else
	{
	  break;
	}
    }

  delete epv;
}

void DeviceClassEditor::slotRemovePresetClicked()
{
  Capability* cap = currentCapability();
  if (cap == NULL)
    {
      return;
    }
  
  currentChannel()->capabilities()->remove(cap);
  delete cap;

  updatePresetValues();

  setModified();
}

void DeviceClassEditor::slotEditPresetClicked()
{
  LogicalChannel* ch = currentChannel();
  Capability* orig = currentCapability();

  if (ch == NULL || orig == NULL)
    {
      return;
    }
  
  EditPresetValue* epv = new EditPresetValue(this);
  epv->m_minSpin->setValue(orig->lo());
  epv->m_maxSpin->setValue(orig->hi());
  epv->m_descriptionEdit->setText(orig->name());

  while(1)
    {
      if (epv->exec() == QDialog::Accepted)
	{
	  int min = epv->m_minSpin->value();
	  int max = epv->m_maxSpin->value();
	  QString description = epv->m_descriptionEdit->text();
	  
	  Capability* minCap = ch->searchCapability(min);
	  Capability* maxCap = ch->searchCapability(max);
	  
	  if (minCap != NULL && minCap != orig)
	    {
	      QMessageBox::information(this, KApplicationNameShort,
				       "Overlapping value range! Unable to change min value.");
	      continue;
	    }
	  else if (maxCap != NULL && maxCap != orig)
	    {
	      QMessageBox::information(this, KApplicationNameShort,
				       "Overlapping value range! Unable to change max value.");
	      continue;;
	    }
	  else
	    {
	      orig->setLo(min);
	      orig->setHi(max);
	      orig->setName(description);
	    }
	  
	  setModified();
	  
	  updatePresetValues();
	  
	  break;
	}
      else
	{
	  break;
	}
    }

  delete epv;
}

void DeviceClassEditor::updateChannelList()
{
  QPtrList <LogicalChannel> *cl = m_dc->channels();
  QString ch;

  m_channelList->clear();

  // Fill channels list
  for (LogicalChannel* c = cl->first(); c != NULL; c = cl->next())
    {
      ch.sprintf("%03d", c->channel() + 1);
      new QListViewItem(m_channelList, ch, c->name());
    }
}

void DeviceClassEditor::updatePresetValues()
{
  m_presetList->clear();

  if (currentChannel() == NULL)
    {
      return;
    }
  else
    {
      QPtrList <Capability> *cl = currentChannel()->capabilities();
      Capability* cap = NULL;

      for (cap = cl->first(); cap != NULL; cap = cl->next())
        {
          QString lo, hi;
          lo.sprintf("%03d", cap->lo());
          hi.sprintf("%03d", cap->hi());
          new QListViewItem(m_presetList, lo, hi, cap->name());
        }
    }
}

LogicalChannel* DeviceClassEditor::currentChannel()
{
  if (m_channelList->currentItem() == NULL)
    {
      return NULL;
    }

  unsigned int channel = 0;
  channel = m_channelList->currentItem()->text(KChannelNumberColumn).toInt()-1;
  ASSERT(channel < m_dc->channels()->count());

  return m_dc->channels()->at(channel);
}

Capability* DeviceClassEditor::currentCapability()
{
  QListViewItem* item = m_presetList->currentItem();
  if (item == NULL)
    {
      return NULL;
    }

  LogicalChannel* channel = currentChannel();
  if (channel != NULL)
    {
      return channel->searchCapability(item->text(KCapabilityMinimumColumn).toInt());
    }
  else
    {
      return NULL;
    }
}
