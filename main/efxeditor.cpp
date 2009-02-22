/*
  Q Light Controller
  efxeditor.cpp

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
#include <QMessageBox>
#include <QPaintEvent>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPainter>
#include <QLabel>
#include <QDebug>
#include <QPen>

#include "common/qlcfixturedef.h"
#include "common/qlcchannel.h"

#include "fixtureselection.h"
#include "efxeditor.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define KColumnNumber  0
#define KColumnName    1
#define KColumnReverse 2
#define KColumnEF      3

#define KInitColumnName     0
#define KInitColumnID       1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

EFXEditor::EFXEditor(QWidget* parent, EFX* efx) : QDialog(parent)
{
	setupUi(this);

	Q_ASSERT(efx != NULL);
	m_original = efx;

	/* Create a copy of the original scene so that we can freely modify it.
	   Keep also a pointer to the original so that we can move the
	   contents from the copied chaser to the original when OK is clicked */
	m_efx = new EFX(this);
	m_efx->copyFrom(efx);
	Q_ASSERT(m_efx != NULL);

	initGeneralPage();
	initMovementPage();
	initInitializationPage();
}

EFXEditor::~EFXEditor()
{
	delete m_efx;
}

void EFXEditor::initGeneralPage()
{
	connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
		this, SLOT(slotNameEdited(const QString&)));

	connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
		this, SLOT(slotFixtureItemChanged(QTreeWidgetItem*,int)));

	connect(m_addFixtureButton, SIGNAL(clicked()),
		this, SLOT(slotAddFixtureClicked()));
	connect(m_removeFixtureButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveFixtureClicked()));

	connect(m_raiseFixtureButton, SIGNAL(clicked()),
		this, SLOT(slotRaiseFixtureClicked()));
	connect(m_lowerFixtureButton, SIGNAL(clicked()),
		this, SLOT(slotLowerFixtureClicked()));

	connect(m_parallelRadio, SIGNAL(toggled(bool)),
		this, SLOT(slotParallelRadioToggled(bool)));

	connect(m_busCombo, SIGNAL(activated(int)),
		this, SLOT(slotBusComboActivated(int)));

	/* Set the EFX's name to the name field */
	m_nameEdit->setText(m_efx->name());
	slotNameEdited(m_efx->name());

	/* Put all of the EFX's fixtures to the tree view */
	QListIterator <EFXFixture*> it(m_efx->fixtures());
	while (it.hasNext() == true)
		addFixtureItem(it.next());

	/* Set propagation mode */
	if (m_efx->propagationMode() == EFX::Serial)
		m_serialRadio->setChecked(true);
	else
		m_parallelRadio->setChecked(true);

	/* Init bus combo and select the EFX's bus */
	fillBusCombo();
}

void EFXEditor::fillBusCombo()
{
	m_busCombo->clear();

	for (t_bus_id i = 0; i < KBusCount; i++)
		m_busCombo->addItem(
			QString("%1: %2").arg(i + 1).arg(Bus::name(i)));

	m_busCombo->setCurrentIndex(m_efx->busID());
}

void EFXEditor::initMovementPage()
{
	m_previewArea = new EFXPreviewArea(m_previewFrame);
	m_previewArea->resize(m_previewFrame->width(),
			      m_previewFrame->height());
	m_efx->setPreviewPointArray(m_previewArea->points());

	/* Get supported algorithms and fill the algorithm combo with them */
	m_algorithmCombo->addItems(EFX::algorithmList());

	connect(m_loop, SIGNAL(clicked()),
		this, SLOT(slotLoopClicked()));
	connect(m_singleShot, SIGNAL(clicked()),
		this, SLOT(slotSingleShotClicked()));
	connect(m_pingPong, SIGNAL(clicked()),
		this, SLOT(slotPingPongClicked()));

	connect(m_forward, SIGNAL(clicked()),
		this, SLOT(slotForwardClicked()));
	connect(m_backward, SIGNAL(clicked()),
		this, SLOT(slotBackwardClicked()));

	connect(m_algorithmCombo, SIGNAL(activated(const QString&)),
		this, SLOT(slotAlgorithmSelected(const QString&)));
	connect(m_widthSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotWidthSpinChanged(int)));
	connect(m_heightSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotHeightSpinChanged(int)));
	connect(m_xOffsetSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotXOffsetSpinChanged(int)));
	connect(m_yOffsetSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotYOffsetSpinChanged(int)));
	connect(m_rotationSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotRotationSpinChanged(int)));

	connect(m_xFrequencySpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotXFrequencySpinChanged(int)));
	connect(m_yFrequencySpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotYFrequencySpinChanged(int)));
	connect(m_xPhaseSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotXPhaseSpinChanged(int)));
	connect(m_yPhaseSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotYPhaseSpinChanged(int)));

	/* Select the EFX's algorithm from the algorithm combo */
	for (int i = 0; i < m_algorithmCombo->count(); i++)
	{
		if (m_algorithmCombo->itemText(i) == m_efx->algorithm())
		{
			m_algorithmCombo->setCurrentIndex(i);
			break;
		}
	}

	/* Causes the EFX function to update the preview point array */
	slotAlgorithmSelected(m_efx->algorithm());

	/* Get the algorithm parameters */
	m_widthSpin->setValue(m_efx->width());
	m_heightSpin->setValue(m_efx->height());
	m_xOffsetSpin->setValue(m_efx->xOffset());
	m_yOffsetSpin->setValue(m_efx->yOffset());
	m_rotationSpin->setValue(m_efx->rotation());
  
	m_xFrequencySpin->setValue(m_efx->xFrequency());
	m_yFrequencySpin->setValue(m_efx->yFrequency());
	m_xPhaseSpin->setValue(m_efx->xPhase());
	m_yPhaseSpin->setValue(m_efx->yPhase());

	/* Running order */
	switch (m_efx->runOrder())
	{
	default:
	case Function::Loop:
		m_loop->setChecked(true);
		break;
	case Function::SingleShot:
		m_singleShot->setChecked(true);
		break;
	case Function::PingPong:
		m_pingPong->setChecked(true);
		break;
	}

	/* Direction */
	switch (m_efx->direction())
	{
	default:
	case Function::Forward:
		m_forward->setChecked(true);
		break;
	case Function::Backward:
		m_backward->setChecked(true);
		break;
	}

	/* Draw the points */
	m_previewArea->repaint();
}

void EFXEditor::initInitializationPage()
{
	connect(m_startSceneGroup, SIGNAL(toggled(bool)),
		this, SLOT(slotStartSceneGroupToggled(bool)));
	connect(m_stopSceneGroup, SIGNAL(toggled(bool)),
		this, SLOT(slotStopSceneGroupToggled(bool)));	
	connect(m_startSceneList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotStartSceneListSelectionChanged()));
	connect(m_stopSceneList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotStopSceneListSelectionChanged()));

	fillSceneLists();
}

void EFXEditor::accept()
{
	m_efx->setName(m_nameEdit->text());

	/* Copy the contents of the modified EFX over the original EFX */
	m_original->copyFrom(m_efx);

	QDialog::accept();
}

/*****************************************************************************
 * General page
 *****************************************************************************/

QTreeWidgetItem* EFXEditor::fixtureItem(EFXFixture* ef)
{
	QTreeWidgetItemIterator it(m_tree);
	while (*it != NULL)
	{
		QTreeWidgetItem* item = *it;
		if (item->text(KColumnEF).toULongLong() == qulonglong(ef))
			return item;
		++it;
	}

	return NULL;
}

const QList <EFXFixture*> EFXEditor::selectedFixtures() const
{
	QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
	QList <EFXFixture*> list;
	
	/* Put all selected fixture IDs to a list and return it */
	while (it.hasNext() == true)
		list << (EFXFixture*) it.next()->text(KColumnEF).toULongLong();

	return list;
}

void EFXEditor::updateIndices(int from, int to)
{
	QTreeWidgetItem* item;
	int i;

	for (i = from; i <= to; i++)
	{
		item = m_tree->topLevelItem(i);
		Q_ASSERT(item != NULL);

		item->setText(KColumnNumber,
			      QString("%1").arg(i + 1, 3, 10, QChar('0')));
	}
}

void EFXEditor::addFixtureItem(EFXFixture* ef)
{
	QTreeWidgetItem* item;
	Fixture* fxi;

	Q_ASSERT(ef != NULL);

	fxi = _app->doc()->fixture(ef->fixture());
	if (fxi == NULL)
		return;

	item = new QTreeWidgetItem(m_tree);
	item->setText(KColumnName, fxi->name());
	item->setText(KColumnEF, QString("%1").arg(qulonglong(ef)));
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

	if (ef->direction() == Function::Backward)
		item->setCheckState(KColumnReverse, Qt::Checked);
	else
		item->setCheckState(KColumnReverse, Qt::Unchecked);

	updateIndices(m_tree->indexOfTopLevelItem(item),
		      m_tree->topLevelItemCount() - 1);

	/* Select newly-added fixtures so that they can be moved quickly */
	m_tree->setCurrentItem(item);
}

void EFXEditor::removeFixtureItem(EFXFixture* ef)
{
	QTreeWidgetItem* item;
	int from;

	Q_ASSERT(ef != NULL);

	item = fixtureItem(ef);
	Q_ASSERT(item != NULL);

	from = m_tree->indexOfTopLevelItem(item);
	delete item;

	updateIndices(from, m_tree->topLevelItemCount() - 1);
}

void EFXEditor::slotNameEdited(const QString &text)
{
	setWindowTitle(tr("EFX - %1").arg(text));
}

void EFXEditor::slotFixtureItemChanged(QTreeWidgetItem* item, int column)
{
	if (column == KColumnReverse)
	{
		EFXFixture* ef;
		ef = (EFXFixture*) item->text(KColumnEF).toULongLong();
		Q_ASSERT(ef != NULL);

		if (item->checkState(column) == Qt::Checked)
			ef->setDirection(Function::Backward);
		else
			ef->setDirection(Function::Forward);
	}
}

void EFXEditor::slotAddFixtureClicked()
{
	/* Put all fixtures already present into a list of fixtures that
	   will be disabled in the fixture selection dialog */
	QList <t_fixture_id> disabled;
	QTreeWidgetItemIterator twit(m_tree);
	while (*twit != NULL)
	{
		EFXFixture* ef;
		ef = (EFXFixture*) ((*twit)->text(KColumnEF).toULongLong());
		Q_ASSERT(ef != NULL);

		/* TODO: Disable all fixtures that don't have pan&tilt chans */

		disabled.append(ef->fixture());
		twit++;
	}

	/* Get a list of new fixtures to add to the scene */
	FixtureSelection fs(this, _app->doc(), true, disabled);
	if (fs.exec() == QDialog::Accepted)
	{
		QListIterator <t_fixture_id> it(fs.selection);
		while (it.hasNext() == true)
		{
			EFXFixture* ef = new EFXFixture(m_efx);
			ef->setFixture(it.next());

			if (m_efx->addFixture(ef) == true)
				addFixtureItem(ef);
			else
				delete ef;
		}
	}
}

void EFXEditor::slotRemoveFixtureClicked()
{
	int r = QMessageBox::question(
		this, tr("Remove fixtures"),
		tr("Do you want to remove the selected fixture(s)?"),
		QMessageBox::Yes, QMessageBox::No);

	if (r == QMessageBox::Yes)
	{
		QListIterator <EFXFixture*> it(selectedFixtures());
		while (it.hasNext() == true)
		{
			EFXFixture* ef = it.next();
			Q_ASSERT(ef != NULL);

			removeFixtureItem(ef);
			if (m_efx->removeFixture(ef) == true)
				delete ef;
		}
	}
}

void EFXEditor::slotRaiseFixtureClicked()
{
	QTreeWidgetItem* item;
	int index;

	item = m_tree->currentItem();
	if (item != NULL)
	{
		EFXFixture* ef;

		index = m_tree->indexOfTopLevelItem(item);
		if (index == 0)
			return;

		ef = (EFXFixture*) (item->text(KColumnEF).toULongLong());
		Q_ASSERT(ef != NULL);

		if (m_efx->raiseFixture(ef) == true)
		{
			item = m_tree->takeTopLevelItem(index);
			m_tree->insertTopLevelItem(index - 1, item);
			m_tree->setCurrentItem(item);

			updateIndices(index - 1, index);
		}
	}
}

void EFXEditor::slotLowerFixtureClicked()
{
	QTreeWidgetItem* item;
	int index;

	item = m_tree->currentItem();
	if (item != NULL)
	{
		EFXFixture* ef;

		index = m_tree->indexOfTopLevelItem(item);
		if (index == (m_tree->topLevelItemCount() - 1))
			return;

		ef = (EFXFixture*) (item->text(KColumnEF).toULongLong());
		Q_ASSERT(ef != NULL);

		if (m_efx->lowerFixture(ef) == true)
		{
			item = m_tree->takeTopLevelItem(index);
			m_tree->insertTopLevelItem(index + 1, item);
			m_tree->setCurrentItem(item);

			updateIndices(index, index + 1);
		}
	}
}

void EFXEditor::slotParallelRadioToggled(bool state)
{
	Q_ASSERT(m_efx != NULL);

	if (state == true)
		m_efx->setPropagationMode(EFX::Parallel);
	else
		m_efx->setPropagationMode(EFX::Serial);
}

void EFXEditor::slotBusComboActivated(int index)
{
	Q_ASSERT(m_efx != NULL);
	m_efx->setBus(index);
}

/*****************************************************************************
 * Movement page
 *****************************************************************************/

void EFXEditor::slotAlgorithmSelected(const QString &text)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setAlgorithm(text);

	if (m_efx->isFrequencyEnabled())
	{
		m_xFrequencyLabel->setEnabled(true);
		m_yFrequencyLabel->setEnabled(true);

		m_xFrequencySpin->setEnabled(true);
		m_yFrequencySpin->setEnabled(true);
	}
	else
	{
		m_xFrequencyLabel->setEnabled(false);
		m_yFrequencyLabel->setEnabled(false);

		m_xFrequencySpin->setEnabled(false);
		m_yFrequencySpin->setEnabled(false);
	}

	if (m_efx->isPhaseEnabled())
	{
		m_xPhaseLabel->setEnabled(true);
		m_yPhaseLabel->setEnabled(true);

		m_xPhaseSpin->setEnabled(true);
		m_yPhaseSpin->setEnabled(true);
	}
	else
	{
		m_xPhaseLabel->setEnabled(false);
		m_yPhaseLabel->setEnabled(false);

		m_xPhaseSpin->setEnabled(false);
		m_yPhaseSpin->setEnabled(false);
	}


	m_previewArea->repaint();
}

void EFXEditor::slotWidthSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setWidth(value);

	m_previewArea->repaint();
}

void EFXEditor::slotHeightSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setHeight(value);

	m_previewArea->repaint();
}

void EFXEditor::slotRotationSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setRotation(value);

	m_previewArea->repaint();
}

void EFXEditor::slotXOffsetSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setXOffset(value);

	m_previewArea->repaint();
}

void EFXEditor::slotYOffsetSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setYOffset(value);

	m_previewArea->repaint();
}

void EFXEditor::slotXFrequencySpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setXFrequency(value);

	m_previewArea->repaint();
}

void EFXEditor::slotYFrequencySpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setYFrequency(value);

	m_previewArea->repaint();
}

void EFXEditor::slotXPhaseSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setXPhase(value);

	m_previewArea->repaint();
}

void EFXEditor::slotYPhaseSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setYPhase(value);

	m_previewArea->repaint();
}

/*****************************************************************************
 * Run order
 *****************************************************************************/

void EFXEditor::slotLoopClicked()
{
	Q_ASSERT(m_efx != NULL);
	m_efx->setRunOrder(Function::Loop);
}

void EFXEditor::slotSingleShotClicked()
{
	Q_ASSERT(m_efx != NULL);
	m_efx->setRunOrder(Function::SingleShot);
}

void EFXEditor::slotPingPongClicked()
{
	Q_ASSERT(m_efx != NULL);
	m_efx->setRunOrder(Function::PingPong);
}

/*****************************************************************************
 * Direction
 *****************************************************************************/

void EFXEditor::slotForwardClicked()
{
	Q_ASSERT(m_efx != NULL);
	m_efx->setDirection(Function::Forward);
}

void EFXEditor::slotBackwardClicked()
{
	Q_ASSERT(m_efx != NULL);
	m_efx->setDirection(Function::Backward);
}

/*****************************************************************************
 * Initialization page
 *****************************************************************************/

void EFXEditor::fillSceneLists()
{
	Function* function;
	QTreeWidgetItem* item;
	QTreeWidgetItem* startItem = NULL;
	QTreeWidgetItem* stopItem = NULL;
	QString s;

	Q_ASSERT(m_efx != NULL);
  
	for (t_function_id id = 0; id < KFunctionArraySize; id++)
	{
		function = _app->doc()->function(id);
		if (function == NULL)
			continue;
	
		if (function->type() == Function::Scene)
		{
			/* Insert the function to start scene list */
			item = new QTreeWidgetItem(m_startSceneList);
			item->setText(KInitColumnName, function->name());
			item->setText(KInitColumnID, s.setNum(function->id()));

			/* Select the scene from the start scene list */
			if (m_efx->startScene() == function->id())
			{
				m_startSceneList->setCurrentItem(item);
				startItem = item;
			}

			/* Insert the function to stop scene list */
			item = new QTreeWidgetItem(m_stopSceneList);
			item->setText(KInitColumnName, function->name());
			item->setText(KInitColumnID, s.setNum(function->id()));

			/* Select the scene from the stop scene list */
			if (m_efx->stopScene() == function->id())
			{
				m_stopSceneList->setCurrentItem(item);
				stopItem = item;
			}
		}
	}
  
	/* Make sure that the selected start scene item is visible */
	if (startItem != NULL)
		m_startSceneList->scrollToItem(startItem);

	/* Make sure that the selected stop scene item is visible */
	if (stopItem != NULL)
		m_stopSceneList->scrollToItem(stopItem);

	/* Start scene enabled status */
	if (m_efx->startSceneEnabled() == true)
		m_startSceneGroup->setChecked(true);
	else
		m_startSceneGroup->setChecked(false);

	/* Stop scene enabled status */
	if (m_efx->stopSceneEnabled() == true)
		m_stopSceneGroup->setChecked(true);
	else
		m_stopSceneGroup->setChecked(false);

	m_stopSceneList->sortItems(KInitColumnName, Qt::AscendingOrder);
	m_startSceneList->sortItems(KInitColumnName, Qt::AscendingOrder);
}

void EFXEditor::slotStartSceneGroupToggled(bool state)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setStartSceneEnabled(state);

	slotStartSceneListSelectionChanged();
}

void EFXEditor::slotStartSceneListSelectionChanged()
{
	Q_ASSERT(m_efx != NULL);

	QTreeWidgetItem* item = m_startSceneList->currentItem();
	if (item != NULL)
		m_efx->setStartScene(item->text(KInitColumnID).toInt());
}

void EFXEditor::slotStopSceneGroupToggled(bool state)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setStopSceneEnabled(state);

	slotStopSceneListSelectionChanged();
}

void EFXEditor::slotStopSceneListSelectionChanged()
{
	Q_ASSERT(m_efx != NULL);
   
	QTreeWidgetItem* item = m_stopSceneList->currentItem();
	if (item != NULL)
		m_efx->setStopScene(item->text(KInitColumnID).toInt());
}

/*****************************************************************************
 * EFX Preview Area implementation
 *****************************************************************************/

/**
 * Constructor
 */
EFXPreviewArea::EFXPreviewArea(QWidget* parent) : QFrame (parent)
{
	QPalette p = palette();
	m_points = new QPolygon();

	setAutoFillBackground(true);
	p.setColor(QPalette::Window, p.color(QPalette::Base));
	setPalette(p);

	setFrameStyle(StyledPanel | Sunken);
}

/**
 * Destructor
 */
EFXPreviewArea::~EFXPreviewArea()
{
	setUpdatesEnabled(false);

	delete m_points;
	m_points = NULL;
}

/**
 * Get the pointer for the point array that is used
 * to draw the preview
 *
 * @return The point array
 */
QPolygon* EFXPreviewArea::points()
{
	return m_points;
}

/**
 * Paint the points in the point array
 */
void EFXPreviewArea::paintEvent(QPaintEvent* e)
{
	QFrame::paintEvent(e);

	QPainter painter(this);
	QPen pen;
	QPoint point;
	QColor color;
	int i;

	/* Crosshairs */
	color = palette().color(QPalette::Mid);
	painter.setPen(color);
	painter.drawLine(127, 0, 127, 255);
	painter.drawLine(0, 127, 255, 127);

	/* Plain points with highlight color */
	color = palette().color(QPalette::Highlight);
	pen.setColor(color);
	painter.setPen(pen);
	painter.drawPolygon(*m_points);

	// Draw the points from the point array
	for (i = 0; updatesEnabled() && i < m_points->size(); i++)
	{
		color = color.lighter(100 + (m_points->size()/100));
		pen.setColor(color);
		painter.setPen(pen);
		point = m_points->point(i);
		painter.drawEllipse(point.x() - 1, point.y() - 1, 2, 2);
	}

	/* Starting point */
	pen.setColor(color);
	painter.setPen(pen);
	point = m_points->point(0);
	painter.fillRect(point.x() - 3, point.y() - 3, 6, 6,
			 QBrush(palette().color(QPalette::Dark)));
}
