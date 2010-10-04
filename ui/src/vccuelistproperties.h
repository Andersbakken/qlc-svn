/*
  Q Light Controller
  vccuelistproperties.h

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

#ifndef VCCUELISTPROPERTIES_H
#define VCCUELISTPROPERTIES_H

#include <QKeySequence>
#include <QDialog>

#include "ui_vccuelistproperties.h"

class VCCueList;

class VCCueListProperties : public QDialog, public Ui_VCCueListProperties
{
    Q_OBJECT

public:
    VCCueListProperties(QWidget* parent, VCCueList* cueList);
    ~VCCueListProperties();

private:
    Q_DISABLE_COPY(VCCueListProperties)

protected slots:
    void slotAddClicked();
    void slotRemoveClicked();
    void slotRaiseClicked();
    void slotLowerClicked();
    void slotAttachClicked();
    void slotDetachClicked();
    void accept();

protected:
    VCCueList* m_cueList;
    QKeySequence m_keySequence;
};

#endif
