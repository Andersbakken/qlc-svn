/*
  Q Light Controller
  monitor.cpp

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

#include <QMdiSubWindow>
#include <QActionGroup>
#include <QPaintEvent>
#include <QFontDialog>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QPainter>
#include <iostream>
#include <QAction>
#include <QTimer>
#include <QColor>
#include <QRect>
#include <QFont>
#include <QMenu>
#include <QIcon>
#include <QtXml>

#include <math.h>

#include "outputmap.h"
#include "monitor.h"
#include "app.h"
#include "doc.h"

#include "common/qlctypes.h"
#include "common/qlcfile.h"

using namespace std;

extern App* _app;

/** Number of pixels to leave between borders and values/labels on X axis */
#define X_OFFSET 10

/** Number of pixels to leave between borders and values/labels on Y axis */
#define Y_OFFSET 5

/*****************************************************************************
 * Initialize
 *****************************************************************************/
Monitor::Monitor(QWidget* parent) : QWidget(parent)
{
	m_universe        = 0;
	m_visibleMin      = 0;
	m_visibleMax      = 0;
	m_newValues       = NULL;
	m_oldValues       = NULL;
	m_timer           = NULL;
	m_updateFrequency = 16;

	init();
}

Monitor::~Monitor()
{
	if (m_timer != NULL)
		delete m_timer;

	while (m_universeActions.isEmpty() == false)
		m_universeActions.takeFirst();

	/* Wait for painter to stop */
	while (m_painter.isActive())
	{
		/* NOP */
	}
		
	delete [] m_newValues;
	delete [] m_oldValues;
}

void Monitor::init()
{
	Q_ASSERT(parentWidget() != NULL);

	/* Master layout */
	new QVBoxLayout(this);

	/* Create menu bar & actions */
	initActions();
	initMenu();

	/* Icon & window title */
	qobject_cast <QMdiSubWindow*> 
		(parentWidget())->setWindowIcon(QIcon(":/monitor.png"));
	qobject_cast <QMdiSubWindow*> 
		(parentWidget())->setWindowTitle("DMX Monitor - Universe 1");
	
	/* Init the arrays that hold the values */
	m_newValues = new t_value[512];
	m_oldValues = new t_value[512];
	for (t_channel i = 0; i < 512; i++)
		m_newValues[i] = m_oldValues[i] = 0;

	/* Start with the first universe */
	setUniverse(0);

	/* Connect and start the update timer with 16Hz */
	setFrequency(16);

	/* Set a reasonable default size */
	parentWidget()->resize(280, 150);
}

void Monitor::initActions()
{
	/* Universe actions */
	QActionGroup* universeGroup = new QActionGroup(this);
	for (t_channel i = 0; i < KUniverseCount; i++)
	{
		QAction* action;
		action = new QAction(QString(tr("Universe %1")).arg(i+1), this);
		m_universeActions.append(action);
		universeGroup->addAction(action);
		action->setCheckable(true);
	}

	/* Font */
	m_fontAction = new QAction(QIcon(":/fonts.png"), tr("Font"), this);
	connect(m_fontAction, SIGNAL(triggered(bool)), this, SLOT(slotFont()));

	QActionGroup* speedGroup = new QActionGroup(this);

	/* 16Hz update speed */
	m_16HzAction = new QAction(tr("16x per second"), this);
	connect(m_16HzAction, SIGNAL(triggered(bool)), this, SLOT(slot16Hz()));
	speedGroup->addAction(m_16HzAction);
	m_16HzAction->setCheckable(true);

	/* 32Hz update speed */
	m_32HzAction = new QAction(tr("32x per second"), this);
	connect(m_32HzAction, SIGNAL(triggered(bool)), this, SLOT(slot32Hz()));
	speedGroup->addAction(m_32HzAction);
	m_32HzAction->setCheckable(true);

	/* 64Hz update speed */
	m_64HzAction = new QAction(tr("64x per second"), this);
	connect(m_64HzAction, SIGNAL(triggered(bool)), this, SLOT(slot64Hz()));
	speedGroup->addAction(m_64HzAction);
	m_64HzAction->setCheckable(true);
}

void Monitor::initMenu()
{
	/* Menu bar */
	Q_ASSERT(layout() != NULL);
	layout()->setMenuBar(new QMenuBar(this));
	
	/* Universe menu */
	QMenu* universeMenu = new QMenu(layout()->menuBar());
	universeMenu->setTitle(tr("Universe"));
	qobject_cast <QMenuBar*> (layout()->menuBar())->addMenu(universeMenu);
	QListIterator <QAction*> it(m_universeActions);
	while (it.hasNext() == true)
		universeMenu->addAction(it.next());

	/* Display menu */
	QMenu* displayMenu = new QMenu(layout()->menuBar());
	displayMenu->setTitle("Display");
	qobject_cast <QMenuBar*> (layout()->menuBar())->addMenu(displayMenu);
	displayMenu->addAction(m_fontAction);
	displayMenu->addSeparator();

	/* Update speed (inside display menu) */
	QMenu* speedMenu = new QMenu(displayMenu);
	displayMenu->addMenu(speedMenu);
	speedMenu->setTitle("Update speed");
	speedMenu->setIcon(QIcon(":/clock.png"));
	speedMenu->addAction(m_16HzAction);
	speedMenu->addAction(m_32HzAction);
	speedMenu->addAction(m_64HzAction);

	connect(universeMenu, SIGNAL(triggered(QAction*)),
		this, SLOT(slotUniverseTriggered(QAction*)));
}

void Monitor::slotUniverseTriggered(QAction* action)
{
	Q_ASSERT(action != NULL);
	setUniverse(action->text().remove("Universe").toInt() - 1);
}

void Monitor::slotFont()
{
	bool ok = false;
	QFont font = QFontDialog::getFont(&ok, m_font, this);
	if (ok == true)
	{
		m_font = font;
		update();
	}
}

void Monitor::slot16Hz()
{
	setFrequency(16);
}

void Monitor::slot32Hz()
{
	setFrequency(32);
}

void Monitor::slot64Hz()
{
	setFrequency(64);
}

void Monitor::setUniverse(t_channel universe)
{
	QString s;
	
	Q_ASSERT(universe < KUniverseCount);
	Q_ASSERT(parentWidget() != NULL);

	m_universeActions.at(universe)->setChecked(true);
	m_universe = universe;
	
	qobject_cast <QMdiSubWindow*> (parentWidget())->setWindowTitle(
		s.sprintf("DMX Monitor - Universe %d", universe + 1));

	update();
}

void Monitor::setFrequency(int freq)
{
	if (freq == 16)
		m_16HzAction->setChecked(true);
	else if (freq == 32)
		m_32HzAction->setEnabled(true);
	else
		m_64HzAction->setEnabled(true);

	m_updateFrequency = freq;

	if (m_timer == NULL)
	{
		m_timer = new QTimer();
		connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
	}
	else
	{
		m_timer->stop();
	}

	m_timer->start(1000 / m_updateFrequency);
}

void Monitor::slotTimeOut()
{
	/* Read only the visible range of values. There's no point in reading
	   such values that aren't going to get drawn anyway. */
	_app->outputMap()->getValueRange(512 * m_universe + m_visibleMin,
					 m_newValues, m_visibleMax + 1);

	// Paint only changed values
	//repaint(false); // This would have told to draw only VALUES
	// TODO Now we draw everything: fixture labels, channel labels & values
	update();
}

/****************************************************************************
 * Label & value painting
 ****************************************************************************/

//
// Paint event has been received
//
void Monitor::paintEvent(QPaintEvent* e)
{
	QFontMetrics metrics(m_font);
	int unitW = metrics.width(QString("000")) + X_OFFSET;
	int unitH = metrics.height();
	int unitsX = (rect().width() - X_OFFSET) / (unitW);
	int y_offset = layout()->menuBar()->height() + Y_OFFSET;
	int x_offset = X_OFFSET;
	
	// Wait until previous painter is finished
	while (m_painter.isActive())
	{
		/* NOP */
	}

	// Begin painting
	m_painter.begin(this);

	// Set the font used for drawing text
	m_painter.setFont(m_font);

	if (1) // e->erased()) TODO
	{
		// Draw everything that is inside the invalidated region
		paintFixtureLabelAll(e->region(), x_offset, y_offset,
				     unitW, unitH, unitsX);
		paintChannelLabelAll(e->region(), x_offset, y_offset,
				     unitW, unitH, unitsX);
		paintChannelValueAll(e->region(), x_offset, y_offset,
				     unitW, unitH, unitsX, false);
	}
	else
	{
		// Draw only changed values
		paintChannelValueAll(e->region(), x_offset, y_offset,
				     unitW, unitH, unitsX, true);
	}

	m_painter.end();
	
	// Paint the generic graphics
	QWidget::paintEvent(e);
}

//
// Paint all visible fixture labels
//
void Monitor::paintFixtureLabelAll(QRegion region, int x_offset, int y_offset,
				   int unitW, int unitH, int unitsX)
{
	int x = 0;
	int y = 0;
	int w = 0;
	int wcur = 0;
	int h = 0;
	
	t_fixture_id id = KNoID;
	Fixture* fxi = NULL;
	
	// Draw fixture names and their channel spaces
	for (id = 0; id < KFixtureArraySize; id++)
	{
		fxi = _app->doc()->fixture(id);
		if (fxi == NULL) continue;
		if (fxi->universe() != m_universe) continue;
		
		// Calculate x and y positions for this fixture label
		x = ((fxi->address() % unitsX) * unitW);
		x += x_offset;
		
		y = static_cast<int> 
			(floor((fxi->address() / unitsX)) * (unitH * 3));
		y += y_offset;
		
		// Get width and height for this fixture label
		w = (fxi->channels() * unitW) - X_OFFSET;
		h = unitH;
			
		// Check if this label needs to be painted at all
		if (region.contains(QRect(x, y, w, h)) == false)
			continue;

		if ((x + w + X_OFFSET) <= width())
		{
			// The label fits to one line, just draw it
			paintFixtureLabel(x, y, w, h, fxi->name());
		}
		else
		{
			// The label needs to be drawn on at least two lines
			while (w > 0)
			{	
				wcur = MIN(w, (unitsX * unitW) - (x));
				
				// Draw the label
				paintFixtureLabel(x, y, wcur, h, fxi->name());
			
				// Calculate remaining width
				w = w - wcur - X_OFFSET;
				
				// Next line
				y += (unitH * 3);
				x = x_offset;
			}
		}
	}
}

//
// Draw the fixture label to the given coordinates
//
void Monitor::paintFixtureLabel(int x, int y, int w, int h, QString label)
{
	m_painter.fillRect(x, y, w, h, palette().color(QPalette::Highlight));

	m_painter.setPen(palette().color(QPalette::Shadow));
	m_painter.drawRect(x, y, w, h);

	m_painter.setPen(palette().color(QPalette::HighlightedText));
	m_painter.drawText(x, y, w, h, Qt::AlignLeft | Qt::AlignVCenter, label);
}

//
// Paint all channel labels
//
void Monitor::paintChannelLabelAll(QRegion region, int x_offset, int y_offset,
				   int unitW, int unitH, int unitsX)
{
	int x = 0;
	int y = 0;
	int i = 0;
	QString s;
	
	for (i = 0; i < 512; i++)
	{
		// Calculate x and y positions for this channel
		x = ((i % unitsX) * (unitW));
		x += x_offset;
		
		y = unitH + static_cast<int> 
			(floor((i / unitsX)) * (unitH * 3));
		y += y_offset;
		
		if (region.contains(QRect(x, y, unitW, unitH)))
		{
			// Paint channel label
			paintChannelLabel(x, y, unitW, unitH,
					  s.sprintf("%.3d", i + 1));
		}
	}
}

//
// Paint channel label
//
void Monitor::paintChannelLabel(int x, int y, int w, int h, QString label)
{
	m_painter.setPen(palette().color(QPalette::WindowText));
	m_painter.eraseRect(x, y, w, h);
	m_painter.drawText(x, y, w, h, Qt::AlignLeft | Qt::AlignVCenter, label);
}

//
// Paint all channel values
//
void Monitor::paintChannelValueAll(QRegion region, int x_offset, int y_offset,
				   int unitW, int unitH, int unitsX,
				   bool onlyDelta)
{
	int x = 0;
	int y = 0;
	int value = 0;
	int i = 0;
	
	QString s;

	// Set normal text color to painter
	m_painter.setPen(palette().color(QPalette::Text));
	
	for (i = 0; i < 512; i++)
	{
		// Lock value array
		m_valueMutex.lock();
	
		if (onlyDelta && m_oldValues[i] == m_newValues[i])
		{
			m_valueMutex.unlock();
			continue;
		}
		
		m_oldValues[i] = m_newValues[i];

		// Get channel value from array;
		value = m_newValues[i];
		
		// Unlock array
		m_valueMutex.unlock();
		
		// Calculate xy position for this channel
		x = ((i % unitsX) * (unitW));
		x += x_offset;
		
		y = (unitH * 2) + 
		    static_cast<int> (floor((i / unitsX)) * (unitH * 3));
		y += y_offset;
		
		// If all values must be drawn, draw only those that are
		// inside the invalidated area, otherwise draw the delta values
		if (!onlyDelta && !region.contains(QRect(x, y, unitW, unitH)))
			continue;
		
		// Draw only those values that are visible
		if (y < height())
		{
			// Convert the value to a string
			s.sprintf("%.3d", value);
		
			// Paint the value
			paintChannelValue(x, y, unitW, unitH, s);

			/* Update the biggest visible channel number only,
			   when the visibility of all channels is being
			   checked */
			if (onlyDelta == false)
				m_visibleMax = i;
		}
		else
		{
			// Rest of the values are not visible either
			break;
		}
	}
}

//
// Paint one channel value
//
void Monitor::paintChannelValue(int x, int y, int w, int h, QString value)
{
	m_painter.eraseRect(x, y, w, h);
	m_painter.drawText(x, y, w, h, Qt::AlignBottom, value);
}

/****************************************************************************
 * Load/save settings
 ****************************************************************************/

void Monitor::loader(QDomDocument*, QDomElement*)
{
	// TODO
	//_app->createMonitor();
	//_app->monitor()->loadXML(doc, root);
}

bool Monitor::loadXML(QDomDocument*, QDomElement* root)
{
	bool visible = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(root != NULL);
	
	if (root->tagName() != KXMLQLCMonitor)
	{
		qWarning("Monitor node not found!");
		return false;
	}

	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCWindowState)
		{
			QLCFile::loadXMLWindowState(&tag, &x, &y, &w, &h,
						    &visible);
		}
		else if (tag.tagName() == KXMLQLCMonitorFont)
		{
			m_font.fromString(tag.text());
			// repaint(true); // Repaint all
			update();
		}
		else if (tag.tagName() == KXMLQLCMonitorUpdateFrequency)
		{
			setFrequency(tag.text().toInt());
		}
		else
		{
			cout << "Unknown monitor tag: "
			     << tag.tagName().toStdString()
			     << endl;
		}
		
		node = node.nextSibling();
	}

	hide();
	setGeometry(x, y, w, h);
	if (visible == false)
		showMinimized();
	else
		showNormal();

	return true;
}

bool Monitor::saveXML(QDomDocument* doc, QDomElement* fxi_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(fxi_root != NULL);

	/* Fixture Console entry */
	root = doc->createElement(KXMLQLCMonitor);
	fxi_root->appendChild(root);

	/* Font */
	tag = doc->createElement(KXMLQLCMonitorFont);
	root.appendChild(tag);
	text = doc->createTextNode(m_font.toString());
	tag.appendChild(text);

	/* Update frequency */
	tag = doc->createElement(KXMLQLCMonitorUpdateFrequency);
	root.appendChild(tag);
	str.setNum(m_updateFrequency);
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Save window state. parentWidget() should be used for all
	   widgets within the workspace. */
	return QLCFile::saveXMLWindowState(doc, &root, parentWidget());
}
