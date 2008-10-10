/*
  Q Light Controller
  vccuelist.cpp

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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QString>
#include <QDebug>
#include <QtXml>

#include "common/qlcfile.h"

#include "vccuelistproperties.h"
#include "virtualconsole.h"
#include "vccuelist.h"
#include "function.h"
#include "keybind.h"
#include "app.h"
#include "doc.h"

extern App* _app;

VCCueList::VCCueList(QWidget* parent) : VCWidget(parent)
{
	/* Set the class name "VCCueList" as the object name as well */
	setObjectName(VCCueList::staticMetaObject.className());

	/* Create a layout for this widget */
	new QVBoxLayout(this);

	/* Create a list for scenes (cues) */
	m_list = new QTreeWidget(this);
	layout()->addWidget(m_list);
	m_list->setSelectionMode(QAbstractItemView::SingleSelection);
	m_list->setAlternatingRowColors(true);
	m_list->setAllColumnsShowFocus(true);
	m_list->setRootIsDecorated(false);
	m_list->setItemsExpandable(false);
	m_list->header()->setSortIndicatorShown(false);
	m_list->header()->setClickable(false);
	m_list->header()->setMovable(false);
	m_list->header()->setResizeMode(QHeaderView::ResizeToContents);

	connect(m_list, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
		this, SLOT(slotItemActivated(QTreeWidgetItem*)));

	setCaption(tr("Cue list"));
	resize(QPoint(200, 200));

	m_keyBind = NULL;
	m_current = NULL;

	slotModeChanged(_app->mode());
}

VCCueList::~VCCueList()
{
}

/*****************************************************************************
 * Cue list
 *****************************************************************************/

void VCCueList::clear()
{
	m_list->clear();
}

void VCCueList::append(t_function_id fid)
{
	QTreeWidgetItem* item;
	Function* function;
	
	function = _app->doc()->function(fid);
	Q_ASSERT(function != NULL);

	item = new QTreeWidgetItem(m_list);
	item->setText(KVCCueListColumnNumber,
		      QString("%1").arg(m_list->indexOfTopLevelItem(item) + 1));
	item->setText(KVCCueListColumnName, function->name());
	item->setText(KVCCueListColumnID, QString("%1").arg(fid));
}

void VCCueList::slotNextCue()
{
	if (_app->mode() != App::Operate)
		return;

	if (m_list->currentItem() == NULL)
	{
		m_list->setCurrentItem(m_list->topLevelItem(0));
	}
	else
	{
		m_list->setCurrentItem(
			m_list->itemBelow(m_list->currentItem()));
	}

	slotItemActivated(m_list->currentItem());
}

void VCCueList::slotFunctionStopped(t_function_id fid)
{
	if (m_current != NULL && m_current->id() == fid)
	{
		disconnect(m_current, SIGNAL(stopped(t_function_id)),
			   this, SLOT(slotFunctionStopped(t_function_id)));
	}
}

void VCCueList::slotItemActivated(QTreeWidgetItem* item)
{
	if (_app->mode() != App::Operate)
		return;
	
	if (m_current != NULL)
		m_current->stop();

	if (item == NULL)
		return;

	m_current = _app->doc()->function(item->text(KVCCueListColumnID).toInt());
	if (m_current == NULL)
		return;

	connect(m_current, SIGNAL(stopped(t_function_id)),
		this, SLOT(slotFunctionStopped(t_function_id)));

	m_current->start();
}

/*****************************************************************************
 * Key Bind
 *****************************************************************************/

void VCCueList::setKeyBind(KeyBind* kb)
{
	if (m_keyBind != NULL)
		delete m_keyBind;

	if (kb != NULL)
	{
		m_keyBind = new KeyBind(kb);
		connect(m_keyBind, SIGNAL(pressed()),
			this, SLOT(slotNextCue()));
	}
	else
	{
		m_keyBind = NULL;
	}
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCCueList::setCaption(const QString& text)
{
	QStringList list;

	VCWidget::setCaption(text);
	m_list->setHeaderLabels(list << tr("Number") << text);
}

/*****************************************************************************
 * QLC Mode
 *****************************************************************************/

void VCCueList::slotModeChanged(App::Mode mode)
{
	if (mode == App::Operate)
	{
		m_list->setEnabled(true);
	}
	else
	{
		m_list->setEnabled(false);
	}

	/* Always start from the beginning */
	m_list->setCurrentItem(NULL);
	m_current = NULL;

	VCWidget::slotModeChanged(mode);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCCueList::slotProperties()
{
	VCCueListProperties prop(_app, this);
	if (prop.exec() == QDialog::Accepted)
		_app->doc()->setModified();
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCCueList::loader(QDomDocument* doc, QDomElement* root, QWidget* parent)
{
	VCCueList* cuelist = NULL;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	Q_ASSERT(parent != NULL);

	if (root->tagName() != KXMLQLCVCCueList)
	{
		qDebug() << "CueList node not found!";
		return false;
	}

	/* Create a new cuelist into its parent */
	cuelist = new VCCueList(parent);
	cuelist->show();

	/* Continue loading */
	return cuelist->loadXML(doc, root);
}

bool VCCueList::loadXML(QDomDocument* doc, QDomElement* root)
{
	bool visible = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	QDomNode node;
	QDomElement tag;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCCueList)
	{
		qDebug() << "CueList node not found!";
		return false;
	}

	/* Caption */
	setCaption(root->attribute(KXMLQLCVCCaption));

	/* Children */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCWindowState)
		{
			QLCFile::loadXMLWindowState(&tag, &x, &y, &w, &h,
						    &visible);
			setGeometry(x, y, w, h);
		}
		else if (tag.tagName() == KXMLQLCVCAppearance)
		{
			loadXMLAppearance(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCKeyBind)
		{
			KeyBind* kb = new KeyBind();
			kb->loadXML(doc, &tag);
			setKeyBind(kb);
		}
		else if (tag.tagName() == KXMLQLCVCCueListFunction)
		{
			append(tag.text().toInt());
		}
		else
		{
			qDebug() << "Unknown cuelist tag:" << tag.tagName();
		}

		node = node.nextSibling();
	}

	return true;
}

bool VCCueList::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	/* VC CueList entry */
	root = doc->createElement(KXMLQLCVCCueList);
	vc_root->appendChild(root);

	/* Caption */
	root.setAttribute(KXMLQLCVCCaption, caption());

	/* Cues */
	QTreeWidgetItemIterator it(m_list);
	while (*it != NULL)
	{
		tag = doc->createElement(KXMLQLCVCCueListFunction);
		root.appendChild(tag);

		text = doc->createTextNode((*it)->text(KVCCueListColumnID));
		tag.appendChild(text);

		++it;
	}

	/* Key binding */
	if (m_keyBind != NULL)
		m_keyBind->saveXML(doc, &root);

	/* Window state */
	QLCFile::saveXMLWindowState(doc, &root, this);

	/* Appearance */
	saveXMLAppearance(doc, &root);

	return true;
}
