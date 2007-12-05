/*
  Q Light Controller
  functioncollectioneditor.h

  Copyright (c) Heikki Junnila

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

#ifndef FUNCTIONCOLLECTIONEDITOR_H
#define FUNCTIONCOLLECTIONEDITOR_H

#include <qdialog.h>
#include "function.h"
#include "uic_functioncollectioneditor.h"

class QLineEdit;
class QLabel;
class QListView;
class QListViewItem;
class QPushButton;
class FunctionManager;

class FunctionCollection;

class FunctionCollectionEditor : public UI_FunctionCollectionEditor
{
	Q_OBJECT

public:
	FunctionCollectionEditor(QWidget* parent, FunctionCollection* fc);
	~FunctionCollectionEditor();

	void init();

protected slots:
	void slotAddFunctionClicked();
	void slotFunctionManagerClosed();
	void slotAddAnother();

	void slotRemoveFunctionClicked();
	void slotOKClicked();
	void slotCancelClicked();

protected:
	bool isAlreadyMember(t_function_id id);
	void updateFunctionList();

protected:
	FunctionCollection* m_fc;
	FunctionCollection* m_original;
	FunctionManager* m_functionManager;
};

#endif

