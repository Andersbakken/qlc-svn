/*
  Q Light Controller
  sequenceeditor.cpp
  
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

#include "sequenceeditor.h"
#include "doc.h"
#include "app.h"

extern App* _app;

SequenceEditor::SequenceEditor(QWidget *parent, const char *name) : QWidget(parent,name)
{
  setCaption("Sequence Editor");
  initDialog();
  
  m_seqnumber = 0;
  
  m_enable[0] = m_enableA;
  m_list[0] = m_listA;
  m_listA->setVScrollBarMode(QScrollView::AlwaysOff);
  m_listA->setHScrollBarMode(QScrollView::AlwaysOff);
  m_listA->clear();
  m_listA->setSorting(-1);
  m_listA->setColumnWidth(0, (m_listA->width() / 3) - 1);
  m_listA->setColumnWidth(1, (m_listA->width() / 3) - 1);
  m_listA->setColumnWidth(2, (m_listA->width() / 3) - 1);
  m_listA->header()->setClickEnabled(false);
  
  m_enable[1] = m_enableB;
  m_list[1] = m_listB;
  m_listB->setVScrollBarMode(QScrollView::AlwaysOff);
  m_listB->setHScrollBarMode(QScrollView::AlwaysOff);
  m_listB->clear();
  m_listB->setSorting(-1);
  m_listB->setColumnWidth(0, (m_listB->width() / 3) - 1);
  m_listB->setColumnWidth(1, (m_listB->width() / 3) - 1);
  m_listB->setColumnWidth(2, (m_listB->width() / 3) - 1);
  m_listB->header()->setClickEnabled(false);
  
  m_enable[2] = m_enableC;
  m_list[2] = m_listC;
  m_listC->setVScrollBarMode(QScrollView::AlwaysOff);
  m_listC->setHScrollBarMode(QScrollView::AlwaysOff);
  m_listC->clear();
  m_listC->setSorting(-1);
  m_listC->setColumnWidth(0, (m_listC->width() / 3) - 1);
  m_listC->setColumnWidth(1, (m_listC->width() / 3) - 1);
  m_listC->setColumnWidth(2, (m_listC->width() / 3) - 1);
  m_listC->header()->setClickEnabled(false);
  
  m_enable[3] = m_enableD;
  m_list[3] = m_listD;
  m_listD->setVScrollBarMode(QScrollView::AlwaysOff);
  m_listD->setHScrollBarMode(QScrollView::AlwaysOff);
  m_listD->clear();
  m_listD->setSorting(-1);
  m_listD->setColumnWidth(0, (m_listD->width() / 3) - 1);
  m_listD->setColumnWidth(1, (m_listD->width() / 3) - 1);
  m_listD->setColumnWidth(2, (m_listD->width() / 3) - 1);
  m_listD->header()->setClickEnabled(false);
  
  m_enable[4] = m_enableE;
  m_list[4] = m_listE;
  m_listE->setVScrollBarMode(QScrollView::AlwaysOff);
  m_listE->setHScrollBarMode(QScrollView::AlwaysOff);
  m_listE->clear();
  m_listE->setSorting(-1);
  m_listE->setColumnWidth(0, (m_listE->width() / 3) - 1);
  m_listE->setColumnWidth(1, (m_listE->width() / 3) - 1);
  m_listE->setColumnWidth(2, (m_listE->width() / 3) - 1);
  m_listE->header()->setClickEnabled(false);
  
  m_enable[5] = m_enableF;
  m_list[5] = m_listF;
  m_listF->setVScrollBarMode(QScrollView::AlwaysOn);
  m_listF->setHScrollBarMode(QScrollView::AlwaysOff);
  m_listF->clear();
  m_listF->setSorting(-1);
  m_listF->setColumnWidth(0, ((m_listF->width() - m_listF->verticalScrollBar()->width()) / 3) - 1);
  m_listF->setColumnWidth(1, ((m_listF->width() - m_listF->verticalScrollBar()->width()) / 3) - 1);
  m_listF->setColumnWidth(2, ((m_listF->width() - m_listF->verticalScrollBar()->width()) / 3) - 1);
  m_listF->header()->setClickEnabled(false);
  
  m_currentItem = 0;
  
  m_scroll->setSteps(1, 6);
  
  // Accept only numbers from 0 to 100 as values
  m_value->setValidator(new QIntValidator(MIN_LIGHT_VALUE, MAX_LIGHT_VALUE, 0));
  
  initContextMenu();
  
  connect(m_add, SIGNAL(clicked()), SLOT(slotAddClicked()));
  connect(m_delete, SIGNAL(clicked()), SLOT(slotDeleteClicked()));
  connect(m_load, SIGNAL(clicked()), SLOT(slotLoadClicked()));
  connect(m_save, SIGNAL(clicked()), SLOT(slotSaveClicked()));
  
  connect(m_play, SIGNAL(clicked()), SLOT(slotPlayClicked()));
  connect(m_loop, SIGNAL(clicked()), SLOT(slotLoopClicked()));
  connect(m_stop, SIGNAL(clicked()), SLOT(slotStopClicked()));
  
  connect(m_seq, SIGNAL(returnPressed()), SLOT(slotSequenceNameReturnPressed()));
  connect(m_seqlist, SIGNAL(highlighted(int)), SLOT(slotSequenceHighlighted(int)));
  
  connect(m_scroll, SIGNAL(valueChanged(int)), SLOT(slotScrollChanged(int)));

  connect(m_value, SIGNAL(textChanged(const QString&)), SLOT(slotValueChanged(const QString&)));
  connect(m_speed, SIGNAL(valueChanged(int)), SLOT(slotSpeedChanged(int)));
  
  connect(m_listA, SIGNAL(selectionChanged(QListViewItem*)), SLOT(slotListAselectionChanged(QListViewItem*)));
  connect(m_listA, SIGNAL(contentsMoving(int, int)), SLOT(slotListAContentsMoving(int, int)));	
  connect(m_enableA, SIGNAL(clicked()), SLOT(slotEnableAClicked()));
  connect(m_listA, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)), SLOT(slotListARightButtonPressed(QListViewItem*, const QPoint&, int)));
  
  connect(m_listB, SIGNAL(selectionChanged(QListViewItem*)), SLOT(slotListBselectionChanged(QListViewItem*)));
  connect(m_listB, SIGNAL(contentsMoving(int, int)), SLOT(slotListBContentsMoving(int, int)));	
  connect(m_enableB, SIGNAL(clicked()), SLOT(slotEnableBClicked()));
  connect(m_listB, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)), SLOT(slotListBRightButtonPressed(QListViewItem*, const QPoint&, int)));
  
  connect(m_listC, SIGNAL(selectionChanged(QListViewItem*)), SLOT(slotListCselectionChanged(QListViewItem*)));
  connect(m_listC, SIGNAL(contentsMoving(int, int)), SLOT(slotListCContentsMoving(int, int)));	
  connect(m_enableC, SIGNAL(clicked()), SLOT(slotEnableCClicked()));
  connect(m_listC, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)), SLOT(slotListCRightButtonPressed(QListViewItem*, const QPoint&, int)));
  
  connect(m_listD, SIGNAL(selectionChanged(QListViewItem*)), SLOT(slotListDselectionChanged(QListViewItem*)));
  connect(m_listD, SIGNAL(contentsMoving(int, int)), SLOT(slotListDContentsMoving(int, int)));
  connect(m_enableD, SIGNAL(clicked()), SLOT(slotEnableDClicked()));
  connect(m_listD, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)), SLOT(slotListDRightButtonPressed(QListViewItem*, const QPoint&, int)));
  
  connect(m_listE, SIGNAL(selectionChanged(QListViewItem*)), SLOT(slotListEselectionChanged(QListViewItem*)));
  connect(m_listE, SIGNAL(contentsMoving(int, int)), SLOT(slotListEContentsMoving(int, int)));
  connect(m_enableE, SIGNAL(clicked()), SLOT(slotEnableEClicked()));
  connect(m_listE, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)), SLOT(slotListERightButtonPressed(QListViewItem*, const QPoint&, int)));
  
  connect(m_listF, SIGNAL(selectionChanged(QListViewItem*)), SLOT(slotListFselectionChanged(QListViewItem*)));
  connect(m_listF, SIGNAL(contentsMoving(int, int)), SLOT(slotListFContentsMoving(int, int)));
  connect(m_enableF, SIGNAL(clicked()), SLOT(slotEnableFClicked()));
  connect(m_listF, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)), SLOT(slotListFRightButtonPressed(QListViewItem*, const QPoint&, int)));
}

SequenceEditor::~SequenceEditor()
{
}

void SequenceEditor::initContextMenu()
{
  m_menu = new QPopupMenu();
  m_menu->insertItem("Cut", ID_SEQLIST_MENU_CUT);
  m_menu->insertItem("Copy", ID_SEQLIST_MENU_COPY);
  m_menu->insertItem("Paste", ID_SEQLIST_MENU_PASTE);
  m_menu->insertSeparator();
  m_menu->insertItem("Snapshot", ID_SEQLIST_MENU_SNAPSHOT);
  
  connect(m_menu, SIGNAL(activated(int)), this, SLOT(slotContextMenuCallback(int)));
}

void SequenceEditor::slotContextMenuCallback(int item)
{
  switch (item)
    {
    case ID_SEQLIST_MENU_CUT:
      break;
    case ID_SEQLIST_MENU_COPY:
      break;
    case ID_SEQLIST_MENU_PASTE:
      break;
    case ID_SEQLIST_MENU_SNAPSHOT:
      break;
    default:
      break;
    }
}
/*
void SequenceEditor::closeEvent(QCloseEvent* event)
{
  _app->slotViewSequenceEditor();
}
*/
void SequenceEditor::show()
{
  m_seqlist->clear();
  /*  
  Sequence *seq;
  for (seq = _app->doc()->m_sequenceList.first(); seq != 0; seq = _app->doc()->m_sequenceList.next())
    {
      m_seqlist->insertItem(seq->m_description);
    }
  */
  m_currentItem = 0;
  
  QWidget::show();
}

void SequenceEditor::slotLoadClicked()
{
  QString fileName = QFileDialog::getOpenFileName(QString::null, "*.seq", this);
  if (!fileName.isEmpty())
    {
      /*
      _app->doc()->loadSequence(fileName);
     */ 
    }
  
  m_seqlist->clear();
  /*  
  Sequence *seq;
  for (seq = _app->doc()->m_sequenceList.first(); seq != 0; seq = _app->doc()->m_sequenceList.next())
    {
      m_seqlist->insertItem(seq->m_description);
    }
  */
  m_currentItem = 0;
  
}

void SequenceEditor::slotSaveClicked()
{
  QString fileName = QFileDialog::getSaveFileName(QString::null, "*.seq", this);
  
  if (!fileName.isEmpty())
    {
      if (fileName.right(3) != QString("seq"))
	{
	  fileName += ".seq";
	}
      /*      
      if (!_app->doc()->saveSequence(fileName, m_sequence->m_description))
	{
	  QMessageBox::warning(this, "Sequence Editor", "Unable to save sequence!");
	}
      */
    }
}

void SequenceEditor::slotSpeedChanged(int value)
{
//  m_sequence->m_speed = value;
}

void SequenceEditor::slotPlayClicked()
{
}

void SequenceEditor::slotLoopClicked()
{
}

void SequenceEditor::slotStopClicked()
{
}

void SequenceEditor::slotValueChanged(const QString& text)
{
  if (m_currentItem == 0 || m_currentPattern == 0) return;
  
  QString	prevValue;
  
  prevValue = m_currentItem->text(2);
  
  if (text.contains(32 /* space clears event */))
    {
      m_value->setText("");
      m_currentItem->setText(2, "---");
      m_currentPattern->data[m_currentItem->text(0).toInt()].data = -1;
      slotCommandHighlighted(0);
    }
  else if (text.isEmpty())
    {
      m_value->setText("");
      m_currentItem->setText(2, "---");
      m_currentPattern->data[m_currentItem->text(0).toInt()].data = -1;
    }
  else if ((text.toInt() < MIN_LIGHT_VALUE) || (text.toInt() > MAX_LIGHT_VALUE))
    {
      m_currentItem->setText(2, prevValue);
    }
  else
    {
      m_currentItem->setText(2, text);
      m_currentPattern->data[m_currentItem->text(0).toInt()].data = text.toInt();
    }
}

void SequenceEditor::slotCommandHighlighted(int index)
{
  if (m_currentItem == 0 || m_currentPattern == 0) return;
  
  m_currentItem->setText(1, getCommandName(index));
  
  m_currentPattern->data[m_currentItem->text(0).toInt()].cmd = index;
}

QString SequenceEditor::getCommandName(int cmd)
{
  QString temp;
  
  switch (cmd)
    {
    case PAT_CMD_FADETO:
      temp = "FTO";
      break;
      
    case PAT_CMD_SET:
      temp = "SET";
      break;
      
    case PAT_CMD_STROBE:
      temp = "STR";
      break;
      
    default:
    case PAT_CMD_NONE:
      temp = "---";
      break;
    }
  
  return temp;
  
}

void SequenceEditor::slotSequenceHighlighted(int index)
{
  if ((index < 0) || (m_seqlist->count() < 1)) return;
  
  Pattern* pat;
  
  m_seq->setText(m_seqlist->text(index));
  /*  
      m_sequence = _app->doc()->searchSequence(m_seqlist->text(index));
  */
  int j = 0;
  for (pat = m_sequence->m_patternList.first(); pat != 0 && j < 6; pat = m_sequence->m_patternList.next())
    {
      m_list[j]->clear();
      m_patterns[j] = pat;
      j+=1;
    }	
  
  m_currentPattern = 0;
  m_currentItem = 0;
  m_speed->setValue(m_sequence->m_speed);
  
  slotScrollChanged(m_scroll->value());
  
  updatePatternLists();
}

void SequenceEditor::updatePatternLists(void)
{
  QListViewItem* item;
  QString	temp;
  
  for (int j = 0; j < 6; j++)
    {
      m_list[j]->clear();
      for (int i = PATTERN_SIZE - 1; i >= 0; i--)
	{
	  item = new QListViewItem(m_list[j]);
	  temp.setNum(i);
	  item->setText(0, temp);
	  
	  item->setText(1, getCommandName(m_patterns[j]->data[i].cmd));
	  
	  if (m_patterns[j]->data[i].data < 0) temp = "---";
	  else temp.setNum(m_patterns[j]->data[i].data);
	  
	  item->setText(2, temp);
	  
	  m_enable[j]->setChecked(m_patterns[j]->m_enabled);
	}
    }
}

void SequenceEditor::zapAll(void)
{
  int i;
  
  m_sequence = 0;
  m_currentPattern = 0;
  
  m_channel->setText("");
  m_position->setText("");
  m_speed->setValue(0);
  
  for (i = 0; i < 6; i++)
    {
      m_enable[i]->setChecked(false);
      m_patterns[i] = 0;
      m_list[i]->clear();
    }
}	

void SequenceEditor::slotAddClicked()
{
  /*
  Sequence*	seq;
  QString	temp;	

  if (!m_seq->text().isEmpty())
    {
      temp = m_seq->text();
            
      // If there already is a sequence[i]
      while (_app->doc()->searchSequence(temp))
  	{
	  temp.sprintf("%s%d", m_seq->text().latin1(), m_seqnumber++);
	}
     
      seq = new Sequence();
      seq->m_description = temp;
      _app->doc()->m_sequenceList.append(seq);
      
      m_seqlist->insertItem(temp, 0);
      m_seqlist->setCurrentItem(0);
      m_seq->setText(temp);
    }
  else
    {
      temp.sprintf("Sequence");
      
      // If there already is a "Sequence"
      while (_app->doc()->searchSequence(temp))
  	{
	  temp.sprintf("Sequence%d", m_seqnumber++);
	}
      
      seq = new Sequence();
      seq->m_description = temp;
      _app->doc()->m_sequenceList.append(seq);
      
      m_seqlist->insertItem(temp, 0);
      m_seqlist->setCurrentItem(0);
      m_seq->setText(temp);
    }

  m_currentPattern = 0;
  slotSequenceHighlighted(0);
  */
}

void SequenceEditor::slotDeleteClicked()
{
  // int prev = m_seqlist->currentItem();
  /*  
  if (m_seqlist->currentItem() >= 0)
    {
      if (_app->doc()->deleteSequence(m_seq->text()))
	{
	  m_seq->clear();
	  m_seqlist->removeItem(m_seqlist->currentItem());
	  zapAll();
	  slotSequenceHighlighted(prev - 1);
	}
    }
  */
}

void SequenceEditor::slotSequenceNameReturnPressed()
{
  if (m_seqlist->currentItem() >= 0)
    {
      Sequence*	sequence = NULL;
      //      sequence = _app->doc()->searchSequence(m_seqlist->text(m_seqlist->currentItem()));
      
      if (sequence)
	{
	  sequence->m_description = m_seq->text();
	  m_seqlist->changeItem(m_seq->text(), m_seqlist->currentItem());
	}
      else
	{
	  QMessageBox::warning(this, "Sequence Editor", "Unable to find sequence!");
	}
    }
}

void SequenceEditor::slotScrollChanged(int value)
{
  Pattern* pat;
  QString	temp;
  
  if (m_sequence == NULL) return;
  
  int j = 0;
  // Skip the ones that are not visible
  for (pat = m_sequence->m_patternList.first(); pat != 0 && j < m_scroll->value(); pat = m_sequence->m_patternList.next())
    {
      j++;
    }
  
  for (j = 0; pat != 0 && j < 6; pat = m_sequence->m_patternList.next())
    {
      m_patterns[j] = pat;
      j+=1;
    }
  
  updatePatternLists();
  
  temp.setNum(value);
  m_numA->setText(temp);
  
  temp.setNum(value + 1);
  m_numB->setText(temp);
  
  temp.setNum(value + 2);
  m_numC->setText(temp);
  
  temp.setNum(value + 3);
  m_numD->setText(temp);
  
  temp.setNum(value + 4);
  m_numE->setText(temp);
  
  temp.setNum(value + 5);
  m_numF->setText(temp);
}


void SequenceEditor::slotListAContentsMoving(int x, int y)
{
  m_listB->setContentsPos(x, y);
  m_listC->setContentsPos(x, y);
  m_listD->setContentsPos(x, y);
  m_listE->setContentsPos(x, y);	
  m_listF->setContentsPos(x, y);
}

void SequenceEditor::slotListBContentsMoving(int x, int y)
{
  m_listA->setContentsPos(x, y);
  m_listC->setContentsPos(x, y);
  m_listD->setContentsPos(x, y);
  m_listE->setContentsPos(x, y);	
  m_listF->setContentsPos(x, y);
}

void SequenceEditor::slotListCContentsMoving(int x, int y)
{
  m_listA->setContentsPos(x, y);
  m_listB->setContentsPos(x, y);
  m_listD->setContentsPos(x, y);
  m_listE->setContentsPos(x, y);	
  m_listF->setContentsPos(x, y);
}

void SequenceEditor::slotListDContentsMoving(int x, int y)
{
  m_listA->setContentsPos(x, y);
  m_listB->setContentsPos(x, y);
  m_listC->setContentsPos(x, y);
  m_listE->setContentsPos(x, y);	
  m_listF->setContentsPos(x, y);
}

void SequenceEditor::slotListEContentsMoving(int x, int y)
{
  m_listA->setContentsPos(x, y);
  m_listB->setContentsPos(x, y);
  m_listC->setContentsPos(x, y);
  m_listD->setContentsPos(x, y);	
  m_listF->setContentsPos(x, y);
}

void SequenceEditor::slotListFContentsMoving(int x, int y)
{
  m_listA->setContentsPos(x, y);
  m_listB->setContentsPos(x, y);
  m_listC->setContentsPos(x, y);
  m_listD->setContentsPos(x, y);
  m_listE->setContentsPos(x, y);	
}

void SequenceEditor::slotEnableAClicked()
{
/*
  if (m_patterns[0] == 0) return;
  
  if (m_enable[0]->isChecked())
    {
      m_patterns[0]->m_enabled = true;
    }
*/
}

void SequenceEditor::slotEnableBClicked()
{
/*
  if (m_patterns[1] == 0) return;
  
  if (m_enable[1]->isChecked())
    {
      m_patterns[1]->m_enabled = true;
    }
*/
}

void SequenceEditor::slotEnableCClicked()
{
/*
  if (m_patterns[2] == 0) return;
  
  if (m_enable[2]->isChecked())
    {
      m_patterns[2]->m_enabled = true;
    }
*/
}

void SequenceEditor::slotEnableDClicked()
{
/*
  if (m_patterns[3] == 0) return;
  
  if (m_enable[3]->isChecked())
    {
      m_patterns[3]->m_enabled = true;
    }
*/
}

void SequenceEditor::slotEnableEClicked()
{
/*
  if (m_patterns[4] == 0) return;
  
  if (m_enable[4]->isChecked())
    {
      m_patterns[4]->m_enabled = true;
    }
*/
}

void SequenceEditor::slotEnableFClicked()
{
/*
  if (m_patterns[5] == 0) return;
  
  if (m_enable[5]->isChecked())
    {
      m_patterns[5]->m_enabled = true;
    }
*/
}

void SequenceEditor::slotListARightButtonPressed(QListViewItem* item, const QPoint& pt, int foo)
{
  m_listA->setSelected(item, true);
  m_menu->exec(pt, 0);
}

void SequenceEditor::slotListBRightButtonPressed(QListViewItem* item, const QPoint& pt, int foo)
{
  m_listB->setSelected(item, true);
  m_menu->exec(pt, 0);
}

void SequenceEditor::slotListCRightButtonPressed(QListViewItem* item, const QPoint& pt, int foo)
{
  m_listC->setSelected(item, true);
  m_menu->exec(pt, 0);
}

void SequenceEditor::slotListDRightButtonPressed(QListViewItem* item, const QPoint& pt, int foo)
{
  m_listD->setSelected(item, true);
  m_menu->exec(pt, 0);
}

void SequenceEditor::slotListERightButtonPressed(QListViewItem* item, const QPoint& pt, int foo)
{
  m_listE->setSelected(item, true);
  m_menu->exec(pt, 0);
}

void SequenceEditor::slotListFRightButtonPressed(QListViewItem* item, const QPoint& pt, int foo)
{
  m_listF->setSelected(item, true);
  m_menu->exec(pt, 0);
}

void SequenceEditor::slotListAselectionChanged(QListViewItem* item)
{
  if (item == NULL) return;
  
  m_listB->clearSelection();
  m_listC->clearSelection();
  m_listD->clearSelection();
  m_listE->clearSelection();
  m_listF->clearSelection();
  
  m_currentPattern = m_patterns[0];
  m_channel->setText(m_numA->text());
  m_position->setText(item->text(0));
  
  m_currentItem = item;
}

void SequenceEditor::slotListBselectionChanged(QListViewItem* item)
{
  if (item == NULL) return;
  
  m_listA->clearSelection();
  m_listC->clearSelection();
  m_listD->clearSelection();
  m_listE->clearSelection();
  m_listF->clearSelection();
  
  m_currentPattern = m_patterns[1];
  m_channel->setText(m_numB->text());
  m_position->setText(item->text(0));
  
  m_currentItem = item;
}

void SequenceEditor::slotListCselectionChanged(QListViewItem* item)
{
  if (item == NULL) return;
  
  m_listA->clearSelection();
  m_listB->clearSelection();
  m_listD->clearSelection();
  m_listE->clearSelection();
  m_listF->clearSelection();
  
  m_currentPattern = m_patterns[2];
  m_channel->setText(m_numC->text());
  m_position->setText(item->text(0));
  
  m_currentItem = item;
}

void SequenceEditor::slotListDselectionChanged(QListViewItem* item)
{
  if (item == NULL) return;
  
  m_listA->clearSelection();
  m_listB->clearSelection();
  m_listC->clearSelection();
  m_listE->clearSelection();
  m_listF->clearSelection();
  
  m_currentPattern = m_patterns[3];
  m_channel->setText(m_numD->text());
  m_position->setText(item->text(0));
  
  m_currentItem = item;
}

void SequenceEditor::slotListEselectionChanged(QListViewItem* item)
{
  if (item == NULL) return;
  
  m_listA->clearSelection();
  m_listB->clearSelection();
  m_listC->clearSelection();
  m_listD->clearSelection();
  m_listF->clearSelection();
  
  m_currentPattern = m_patterns[4];
  m_channel->setText(m_numE->text());
  m_position->setText(item->text(0));
  
  m_currentItem = item;
}

void SequenceEditor::slotListFselectionChanged(QListViewItem* item)
{
  if (item == NULL) return;
  
  m_listA->clearSelection();
  m_listB->clearSelection();
  m_listC->clearSelection();
  m_listD->clearSelection();
  m_listE->clearSelection();
  
  m_currentPattern = m_patterns[5];
  m_channel->setText(m_numF->text());
  m_position->setText(item->text(0));
  
  m_currentItem = item;
}

void SequenceEditor::initDialog()
{
  resize(630, 500);
  setMinimumSize(630, 500);
  setMaximumSize(630, 500);

  qgroupbox1= new QGroupBox(this,"NoName");
  qgroupbox1->setGeometry(10,10,410,50);
  qgroupbox1->setMinimumSize(0,0);
  qgroupbox1->setTitle("Sequence");

  qlabel1= new QLabel(this,"NoName");
  qlabel1->setGeometry(20,30,80,20);
  qlabel1->setMinimumSize(0,0);
  qlabel1->setText("Speed");

  m_speed= new QSpinBox(this,"NoName");
  m_speed->setGeometry(100,30,40,20);
  m_speed->setMinimumSize(0,0);
  m_speed->setRange(0,99);

  m_progress= new QProgressBar(this,"NoName");
  m_progress->setGeometry(170,30,100,20);
  m_progress->setTotalSteps(64);

  m_enableA= new QCheckBox(this,"NoName");
  m_enableA->setGeometry(50,160,20,30);
  m_enableA->setText("");

  m_listA= new QListView(this,"NoName");
  m_listA->setGeometry(10,190,100,250);
  m_listA->setMultiSelection(false);
  m_listA->setAllColumnsShowFocus(true);
  m_listA->setRootIsDecorated(false);
  m_listA->addColumn("#");
  m_listA->addColumn("Val");

  m_numA= new QLabel(this,"NoName");
  m_numA->setGeometry(10,440,100,20);
  m_numA->setText("00");

  m_enableB= new QCheckBox(this,"NoName");
  m_enableB->setGeometry(150,160,20,30);
  m_enableB->setText("");

  m_listB= new QListView(this,"NoName");
  m_listB->setGeometry(110,190,100,250);
  m_listB->setMultiSelection(false);
  m_listB->setAllColumnsShowFocus(true);
  m_listB->setRootIsDecorated(false);
  m_listB->addColumn("#");
  m_listB->addColumn("Val");

  m_numB= new QLabel(this,"NoName");
  m_numB->setGeometry(110,440,100,20);
  m_numB->setText("01");

  m_enableC= new QCheckBox(this,"NoName");
  m_enableC->setGeometry(250,160,20,30);
  m_enableC->setText("");

  m_listC= new QListView(this,"NoName");
  m_listC->setGeometry(210,190,100,250);
  m_listC->setMultiSelection(false);
  m_listC->setAllColumnsShowFocus(true);
  m_listC->setRootIsDecorated(false);
  m_listC->addColumn("#");
  m_listC->addColumn("Val");

  m_numC= new QLabel(this,"NoName");
  m_numC->setGeometry(210,440,100,20);
  m_numC->setText("02");

  m_enableD= new QCheckBox(this,"NoName");
  m_enableD->setGeometry(350,160,20,30);
  m_enableD->setText("");

  m_listD= new QListView(this,"NoName");
  m_listD->setGeometry(310,190,100,250);
  m_listD->setMultiSelection(false);
  m_listD->setAllColumnsShowFocus(true);
  m_listD->setRootIsDecorated(false);
  m_listD->addColumn("#");
  m_listD->addColumn("Val");

  m_numD= new QLabel(this,"NoName");
  m_numD->setGeometry(310,440,100,20);
  m_numD->setText("03");

  m_enableE= new QCheckBox(this,"NoName");
  m_enableE->setGeometry(450,160,20,30);
  m_enableE->setText("");

  m_listE= new QListView(this,"NoName");
  m_listE->setGeometry(410,190,100,250);
  m_listE->setMultiSelection(false);
  m_listE->setAllColumnsShowFocus(true);
  m_listE->setRootIsDecorated(false);
  m_listE->addColumn("#");
  m_listE->addColumn("Val");

  m_numE= new QLabel(this,"NoName");
  m_numE->setGeometry(410,440,100,20);
  m_numE->setText("04");

  m_enableF= new QCheckBox(this,"NoName");
  m_enableF->setGeometry(550,160,20,30);
  m_enableF->setText("");

  m_listF= new QListView(this,"NoName");
  m_listF->setGeometry(510,190,110,250);
  m_listF->setMultiSelection(false);
  m_listF->setAllColumnsShowFocus(true);
  m_listF->setRootIsDecorated(false);
  m_listF->addColumn("#");
  m_listF->addColumn("Val");

  m_numF= new QLabel(this,"NoName");
  m_numF->setGeometry(510,440,100,20);
  m_numF->setText("05");

  m_scroll= new QScrollBar(this,"NoName");
  m_scroll->setGeometry(10,470,610,20);
  m_scroll->setRange(0,58);
  m_scroll->setOrientation(QScrollBar::Horizontal);

  qgroupbox2= new QGroupBox(this,"NoName");
  qgroupbox2->setGeometry(10,60,410,90);
  qgroupbox2->setTitle("Event");

  qlabel2= new QLabel(this,"NoName");
  qlabel2->setGeometry(20,80,80,20);
  qlabel2->setText("Channel");

  m_channel= new QLineEdit(this,"NoName");
  m_channel->setGeometry(100,80,40,20);
  m_channel->setEnabled(false);
  m_channel->setText("");

  qlabel3= new QLabel(this,"NoName");
  qlabel3->setGeometry(20,110,80,20);
  qlabel3->setText("Position");

  m_position= new QLineEdit(this,"NoName");
  m_position->setGeometry(100,110,40,20);
  m_position->setEnabled(false);
  m_position->setText("");

  qlabel4= new QLabel(this,"NoName");
  qlabel4->setGeometry(170,80,80,20);
  qlabel4->setText("Command");

  m_value= new QLineEdit(this,"NoName");
  m_value->setGeometry(270,110,120,20);
  m_value->setText("");

  qlabel5= new QLabel(this,"NoName");
  qlabel5->setGeometry(170,110,80,20);
  qlabel5->setText("Value");

  m_seq= new QLineEdit(this,"NoName");
  m_seq->setGeometry(430,130,160,20);
  m_seq->setText("");

  qlabel6= new QLabel(this,"NoName");
  qlabel6->setGeometry(430,10,100,20);
  qlabel6->setText("Sequence list");

  m_seqlist= new QListBox(this,"NoName");
  m_seqlist->setGeometry(430,30,160,100);

  m_add= new QPushButton(this,"NoName");
  m_add->setGeometry(600,40,30,23);
  m_add->setText("+");

  m_delete= new QPushButton(this,"NoName");
  m_delete->setGeometry(600,60,30,23);
  m_delete->setText("-");

  m_load= new QPushButton(this,"NoName");
  m_load->setGeometry(600,100,30,23);
  m_load->setText("L");

  m_save= new QPushButton(this,"NoName");
  m_save->setGeometry(600,120,30,23);
  m_save->setText("S");

  m_play= new QPushButton(this,"NoName");
  m_play->setGeometry(270,30,40,23);
  m_play->setText("Play");

  m_loop= new QPushButton(this,"NoName");
  m_loop->setGeometry(310,30,40,23);
  m_loop->setText("Loop");
  m_loop->setToggleButton(true);

  m_stop= new QPushButton(this,"NoName");
  m_stop->setGeometry(350,30,40,23);
  m_stop->setText("Stop");
}
