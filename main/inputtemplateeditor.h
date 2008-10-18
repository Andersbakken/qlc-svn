/*
  Q Light Controller
  inputtemplateeditor.h

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

#ifndef INPUTTEMPLATEEDITOR_H
#define INPUTTEMPLATEEDITOR_H

#include <QDialog>
#include "ui_inputtemplateeditor.h"

class QLCInputChannel;
class QLCInputDevice;

class InputTemplateEditor : public QDialog, public Ui_InputTemplateEditor
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	InputTemplateEditor(QWidget* parent, QLCInputDevice* deviceTemplate);
	virtual ~InputTemplateEditor();

protected:
	void fillTree();
	void updateChannelItem(QTreeWidgetItem* item, QLCInputChannel* ch);

	/********************************************************************
	 * OK & Cancel
	 ********************************************************************/
public slots:
	void accept();

	/********************************************************************
	 * Editing
	 ********************************************************************/
protected slots:
	void slotAddClicked();
	void slotRemoveClicked();
	void slotEditClicked();
	void slotWizardClicked();

	/********************************************************************
	 * Template
	 ********************************************************************/
public:
	const QLCInputDevice* deviceTemplate() const;

protected:
	QLCInputDevice* m_deviceTemplate;
};

#endif
