/*
  Q Light Controller
  function_stub.cpp

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

#include "function_stub.h"

Function_Stub::Function_Stub(QObject* parent) : Function(parent)
{
	m_returnState = true;
	m_writeCalls = 0;
}

Function_Stub::~Function_Stub()
{
}

Function* Function_Stub::createCopy(Doc* parent)
{
	Q_UNUSED(parent);
	return NULL;
}

Function::Type Function_Stub::type() const
{
	return Function::Type(0xDEADBEEF);
}

bool Function_Stub::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	Q_UNUSED(doc);
	Q_UNUSED(wksp_root);

	return false;
}

bool Function_Stub::loadXML(const QDomElement* root)
{
	Q_UNUSED(root);
	return false;
}

void Function_Stub::arm()
{
}

void Function_Stub::disarm()
{
}

void Function_Stub::setReturnState(bool state)
{
	m_returnState = state;
}

bool Function_Stub::write(QByteArray* universes)
{
	Q_UNUSED(universes);

	m_writeCalls++;
	return m_returnState;
}

