/*
  Q Light Controller
  sequenceeditor.h
  
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

#ifndef SEQUENCEEDITOR_H
#define SEQUENCEEDITOR_H

#include "track.h"
#include "sequence.h"
#include "pattern.h"
#include "classes.h"

#include <qwidget.h>
#include <qgroupbox.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qscrollbar.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qprogressbar.h>

#include <qpopupmenu.h>
#include <stdlib.h>
#include <qvalidator.h>
#include <qfiledialog.h>
#include <qwidget.h>
#include <qheader.h>

#define ID_SEQLIST_MENU_CUT       15001
#define ID_SEQLIST_MENU_COPY      15002
#define ID_SEQLIST_MENU_PASTE     15003
#define ID_SEQLIST_MENU_SNAPSHOT  15004

class SequenceEditor : public QWidget
{
  Q_OBJECT
 public: 
  SequenceEditor(QWidget *parent=0, const char *name=0);
  ~SequenceEditor();
  
  void show();
  
 protected:
  void initDialog();
  
  QGroupBox *qgroupbox1;
  QGroupBox *qgroupbox2;

  QListView *m_listA;
  QListView *m_listB;
  QListView *m_listC;
  QListView *m_listD;
  QListView *m_listE;
  QListView *m_listF;

  QCheckBox *m_enableA;
  QCheckBox *m_enableB;
  QCheckBox *m_enableC;
  QCheckBox *m_enableD;
  QCheckBox *m_enableE;
  QCheckBox *m_enableF;

  QLabel *m_numA;
  QLabel *m_numB;
  QLabel *m_numC;
  QLabel *m_numD;
  QLabel *m_numE;
  QLabel *m_numF;

  QScrollBar *m_scroll;

  QLineEdit *m_channel;
  QLineEdit *m_position;
  QLineEdit *m_value;
  QProgressBar *m_progress;

  QLabel *qlabel1;
  QLabel *qlabel2;
  QLabel *qlabel3;
  QLabel *qlabel4;
  QLabel *qlabel5;
  QLabel *qlabel6;

  QSpinBox *m_speed;

  QLineEdit *m_seq;
  QListBox *m_seqlist;

  QPushButton *m_play;
  QPushButton *m_stop;
  QPushButton *m_loop;

  QPushButton *m_delete;
  QPushButton *m_add;

  QPushButton *m_load;
  QPushButton *m_save;

//  void closeEvent(QCloseEvent* event);
  
 public slots:
  void slotAddClicked();
  void slotDeleteClicked();
  void slotLoadClicked();
  void slotSaveClicked();
  void slotSequenceNameReturnPressed();
  void slotSequenceHighlighted(int);
  
  void slotCommandHighlighted(int);
  void slotValueChanged(const QString&);
  void slotSpeedChanged(int);
  void slotPlayClicked();
  void slotLoopClicked();
  void slotStopClicked();
  void slotScrollChanged(int);
  
  void slotListARightButtonPressed(QListViewItem* item, const QPoint& pt, int foo);
  void slotListBRightButtonPressed(QListViewItem* item, const QPoint& pt, int foo);
  void slotListCRightButtonPressed(QListViewItem* item, const QPoint& pt, int foo);
  void slotListDRightButtonPressed(QListViewItem* item, const QPoint& pt, int foo);
  void slotListERightButtonPressed(QListViewItem* item, const QPoint& pt, int foo);
  void slotListFRightButtonPressed(QListViewItem* item, const QPoint& pt, int foo);
  
  void slotListAselectionChanged(QListViewItem*);
  void slotListBselectionChanged(QListViewItem*);
  void slotListCselectionChanged(QListViewItem*);
  void slotListDselectionChanged(QListViewItem*);
  void slotListEselectionChanged(QListViewItem*);
  void slotListFselectionChanged(QListViewItem*);
  
  void slotListAContentsMoving(int, int);
  void slotListBContentsMoving(int, int);
  void slotListCContentsMoving(int, int);
  void slotListDContentsMoving(int, int);
  void slotListEContentsMoving(int, int);
  void slotListFContentsMoving(int, int);
  
  void slotEnableAClicked();
  void slotEnableBClicked();
  void slotEnableCClicked();
  void slotEnableDClicked();
  void slotEnableEClicked();
  void slotEnableFClicked();
  
  void slotContextMenuCallback(int item);
  
 private: 
  QListView* m_list[6];
  QCheckBox* m_enable[6];
  
  Sequence* m_sequence;
  
  Pattern* m_patterns[6];
  Pattern* m_currentPattern;

  QListViewItem* m_currentItem;
  
  void updatePatternLists(void);
  
  void zapAll(void);
  void slotCommandhighlighted(int);
  void initContextMenu(void);
  
  QString getCommandName(int);
  
  int m_seqnumber;
  
  QPopupMenu* m_menu;
};

#endif
