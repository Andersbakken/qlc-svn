/*
  Q Light Controller
  addvcbuttonmatrix.h

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

#ifndef ADDVCBUTTONMATRIX_H
#define ADDVCBUTTONMATRIX_H

#include <QDialog>

#include "ui_addvcbuttonmatrix.h"
#include "qlctypes.h"

class AddVCButtonMatrix : public QDialog, public Ui_AddVCButtonMatrix
{
    Q_OBJECT

public:
    AddVCButtonMatrix(QWidget* parent);
    ~AddVCButtonMatrix();

public:
    enum FrameStyle
    {
        NormalFrame = 0,
        SoloFrame
    };

    QList <t_function_id> functions() const {
        return m_functions;
    }
    quint32 horizontalCount() const {
        return m_horizontalCount;
    }
    quint32 verticalCount() const {
        return m_verticalCount;
    }
    quint32 buttonSize() const {
        return m_buttonSize;
    }
    FrameStyle frameStyle() const {
        return m_frameStyle;
    }

protected slots:
    void slotAddClicked();
    void slotRemoveClicked();
    void slotHorizontalChanged();
    void slotVerticalChanged();
    void slotButtonSizeChanged();
    void slotNormalFrameToggled(bool toggled);
    void accept();

private:
    void addFunction(t_function_id fid);
    void setAllocationText();
    void setFrameStyle(FrameStyle style);

private:
    QList <t_function_id> m_functions;
    quint32 m_horizontalCount;
    quint32 m_verticalCount;
    quint32 m_buttonSize;
    FrameStyle m_frameStyle;
};

#endif
