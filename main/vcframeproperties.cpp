/*
  Q Light Controller
  vcframeproperties.cpp
  
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

#include <QRadioButton>

#include "vcframeproperties.h"
#include "vcframe.h"

VCFrameProperties::VCFrameProperties(QWidget* parent, VCFrame* frame)
	: QDialog(parent)
{
	Q_ASSERT(frame != NULL);
	m_frame = frame;

	setupUi(this);

	if (m_frame->buttonBehaviour() == VCFrame::Exclusive)
		m_exclusive->setChecked(true);
	else
		m_normal->setChecked(true);
}

VCFrameProperties::~VCFrameProperties()
{
}

void VCFrameProperties::accept()
{
	if (m_exclusive->isChecked() == true)
		m_frame->setButtonBehaviour(VCFrame::Exclusive);
	else
		m_frame->setButtonBehaviour(VCFrame::Normal);
	QDialog::accept();
}
