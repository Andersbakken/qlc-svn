/*
  Q Light Controller
  chasereditor.h

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

#ifndef CHASEREDITOR_H
#define CHASEREDITOR_H

#include <qdialog.h>
#include "uic_chasereditor.h"

class Chaser;
class FunctionManager;

class ChaserEditor : public UI_ChaserEditor
{
	Q_OBJECT

public:
	ChaserEditor(QWidget* parent, Chaser* chaser);
	~ChaserEditor();

protected:
	void updateStepList();
	void updateOrderNumbers();

protected slots:
	void slotOKClicked();
	void slotCancelClicked();

	void slotAddClicked();
	void slotAddAnother();
	void slotRemoveClicked();
	void slotFunctionManagerClosed();

	void slotPlayClicked();
	void slotRaiseClicked();
	void slotLowerClicked();

protected:
	Chaser* m_chaser;
	Chaser* m_original;
	FunctionManager* m_functionManager;
};

#endif
