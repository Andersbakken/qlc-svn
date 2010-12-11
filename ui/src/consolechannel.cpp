/*
  Q Light Controller
  consolechannel.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Versio 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QContextMenuEvent>
#include <QIntValidator>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLineEdit>
#include <QSlider>
#include <QLabel>
#include <QMenu>
#include <QList>
#include <QtXml>

#include "qlcchannel.h"
#include "qlccapability.h"

#include "app.h"
#include "doc.h"
#include "fixture.h"
#include "outputmap.h"
#include "mastertimer.h"
#include "universearray.h"
#include "consolechannel.h"

extern App* _app;

#define CMARGIN_LEFT      1
#define CMARGIN_TOP       1
#define CMARGIN_TOP_CHECK 15 /* Leave some space for the check box */
#define CMARGIN_RIGHT     1
#define CMARGIN_BOTTOM    1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConsoleChannel::ConsoleChannel(QWidget* parent, t_fixture_id fixtureID,
                               quint32 channel) : QGroupBox(parent)
{
    /* Set the class name as the object name */
    setObjectName(ConsoleChannel::staticMetaObject.className());

    Q_ASSERT(fixtureID != Fixture::invalidId());
    m_fixtureID = fixtureID;

    // Check that we have an actual fixture
    m_fixture = _app->doc()->fixture(m_fixtureID);
    Q_ASSERT(m_fixture != NULL);

    // Check that the given channel is valid
    Q_ASSERT(channel != QLCChannel::invalid());
    Q_ASSERT(channel < m_fixture->channels());
    m_channel = channel;

    m_value = 0;
    m_valueChanged = false;
    m_menu = NULL;
    m_outputDMX = true;

    m_presetButton = NULL;
    m_validator = NULL;
    m_valueEdit = NULL;
    m_valueSlider = NULL;
    m_numberLabel = NULL;

    setMinimumWidth(50);

    init();

    setStyle(App::saneStyle());
}

ConsoleChannel::~ConsoleChannel()
{
    _app->masterTimer()->unregisterDMXSource(this);
}

void ConsoleChannel::init()
{
    setCheckable(true);
    connect(this, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));

    new QVBoxLayout(this);
    layout()->setAlignment(Qt::AlignHCenter);
    layout()->setContentsMargins(CMARGIN_LEFT, CMARGIN_TOP_CHECK,
                                 CMARGIN_RIGHT, CMARGIN_BOTTOM);

    /* Create a button only if its menu has sophisticated contents */
    if (m_fixture->fixtureDef() != NULL && m_fixture->fixtureMode() != NULL)
    {
        m_presetButton = new QToolButton(this);
        m_presetButton->setStyle(App::saneStyle());
        m_presetButton->setIconSize(QSize(26, 26));
        m_presetButton->setMinimumSize(QSize(32, 32));
        layout()->addWidget(m_presetButton);
        layout()->setAlignment(m_presetButton, Qt::AlignHCenter);
        initMenu();
    }

    m_valueEdit = new QLineEdit(this);
    m_valueEdit->setText(QString::number(0));
    layout()->addWidget(m_valueEdit);
    m_valueEdit->setAlignment(Qt::AlignCenter);
    m_validator = new QIntValidator(0, UCHAR_MAX, this);
    m_valueEdit->setValidator(m_validator);
    m_valueEdit->setMinimumSize(QSize(1, 1));

    m_valueSlider = new QSlider(this);
    m_valueSlider->setStyle(App::saneStyle());
    layout()->addWidget(m_valueSlider);
    m_valueSlider->setInvertedAppearance(false);
    m_valueSlider->setRange(0, UCHAR_MAX);
    m_valueSlider->setPageStep(1);
    m_valueSlider->setSizePolicy(QSizePolicy::Preferred,
                                 QSizePolicy::Expanding);

    m_numberLabel = new QLabel(this);
    layout()->addWidget(m_numberLabel);
    m_numberLabel->setAlignment(Qt::AlignCenter);

    // Generic fixtures don't have channel objects
    const QLCChannel* ch = m_fixture->channel(m_channel);
    if (ch != NULL)
        setToolTip(QString("%1").arg(ch->name()));
    else
        setToolTip(tr("Level"));

    // Set channel label
    m_numberLabel->setText(QString::number(m_channel + 1));

    connect(m_valueEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotValueEdited(const QString&)));
    connect(m_valueSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotValueChange(int)));

    /* Register this object as a source of DMX data */
    _app->masterTimer()->registerDMXSource(this);
}

/*****************************************************************************
 * Menu
 *****************************************************************************/

void ConsoleChannel::initMenu()
{
    const QLCChannel* ch;
    QAction* action;

    Q_ASSERT(m_fixture != NULL);

    ch = m_fixture->channel(m_channel);
    Q_ASSERT(ch != NULL);

    // Get rid of a possible previous menu
    if (m_menu != NULL)
    {
        delete m_menu;
        m_menu = NULL;
    }

    // Create a popup menu and set the channel name as its title
    m_menu = new QMenu(this);
    m_presetButton->setMenu(m_menu);
    m_presetButton->setPopupMode(QToolButton::InstantPopup);

    switch(ch->group())
    {
    case QLCChannel::Pan:
        m_presetButton->setIcon(QIcon(":/pan.png"));
        break;
    case QLCChannel::Tilt:
        m_presetButton->setIcon(QIcon(":/tilt.png"));
        break;
    case QLCChannel::Colour:
        setColourButton(ch);
        break;
    case QLCChannel::Effect:
        m_presetButton->setIcon(QIcon(":/efx.png"));
        break;
    case QLCChannel::Gobo:
        m_presetButton->setIcon(QIcon(":/gobo.png"));
        break;
    case QLCChannel::Shutter:
        m_presetButton->setIcon(QIcon(":/shutter.png"));
        break;
    case QLCChannel::Speed:
        m_presetButton->setIcon(QIcon(":/speed.png"));
        break;
    case QLCChannel::Prism:
        m_presetButton->setIcon(QIcon(":/prism.png"));
        break;
    case QLCChannel::Maintenance:
        m_presetButton->setIcon(QIcon(":/maintenance.png"));
        break;
    case QLCChannel::Intensity:
        setColourButton(ch);
        break;
    case QLCChannel::Beam:
        m_presetButton->setIcon(QIcon(":/beam.png"));
        break;
    default:
        m_presetButton->setText("?");
        break;
    }

    action = m_menu->addAction(m_presetButton->icon(), ch->name());
    m_menu->setTitle(ch->name());
    action->setEnabled(false);
    m_menu->addSeparator();

    // Initialize the preset menu only for normal fixtures,
    // i.e. not for Generic dimmer fixtures
    if (m_fixture->fixtureDef() != NULL && m_fixture->fixtureMode() != NULL)
        initCapabilityMenu(ch);
}

void ConsoleChannel::setColourButton(const QLCChannel* channel)
{
    if (channel->name().contains("red", Qt::CaseInsensitive) == true)
    {
        QPalette pal = m_presetButton->palette();
        pal.setColor(QPalette::Button, Qt::red);
        m_presetButton->setPalette(pal);
        m_presetButton->setText("R"); // Don't localize
    }
    else if (channel->name().contains("green", Qt::CaseInsensitive) == true)
    {
        QPalette pal = m_presetButton->palette();
        pal.setColor(QPalette::Button, Qt::green);
        m_presetButton->setPalette(pal);
        m_presetButton->setText("G"); // Don't localize
    }
    else if (channel->name().contains("blue", Qt::CaseInsensitive) == true)
    {
        QPalette pal = m_presetButton->palette();
        pal.setColor(QPalette::Button, Qt::blue);
        pal.setColor(QPalette::ButtonText, Qt::white); // Improve contrast
        m_presetButton->setPalette(pal);
        m_presetButton->setText("B"); // Don't localize
    }
    else if (channel->name().contains("cyan", Qt::CaseInsensitive) == true)
    {
        QPalette pal = m_presetButton->palette();
        pal.setColor(QPalette::Button, QColor("cyan"));
        m_presetButton->setPalette(pal);
        m_presetButton->setText("C"); // Don't localize
    }
    else if (channel->name().contains("magenta", Qt::CaseInsensitive) == true)
    {
        QPalette pal = m_presetButton->palette();
        pal.setColor(QPalette::Button, QColor("magenta"));
        m_presetButton->setPalette(pal);
        m_presetButton->setText("M"); // Don't localize
    }
    else if (channel->name().contains("yellow", Qt::CaseInsensitive) == true)
    {
        QPalette pal = m_presetButton->palette();
        pal.setColor(QPalette::Button, QColor("yellow"));
        m_presetButton->setPalette(pal);
        m_presetButton->setText("Y"); // Don't localize
    }
    else if (channel->group() == QLCChannel::Colour)
    {
        // None of the primary colour names matched, but since this is still
        // a colour channel, it must be controlling a fixed color wheel
        m_presetButton->setIcon(QIcon(":/color.png"));
    }
    else if (channel->group() == QLCChannel::Intensity)
    {
        // None of the primary colour names matched and since this is an
        // intensity channel, it must be controlling a plain dimmer OSLT.
        m_presetButton->setIcon(QIcon(":/intensity.png"));
    }
}

void ConsoleChannel::initCapabilityMenu(const QLCChannel* ch)
{
    QLCCapability* cap;
    QMenu* valueMenu;
    QAction* action;
    QString s;
    QString t;

    QListIterator <QLCCapability*> it(ch->capabilities());
    while (it.hasNext() == true)
    {
        cap = it.next();

        // Set the value range and name as the menu item's name
        s = QString("%1: %2 - %3").arg(cap->name())
            .arg(cap->min()).arg(cap->max());

        if (cap->max() - cap->min() > 0)
        {
            // Create submenu for ranges of more than one value
            valueMenu = new QMenu(m_menu);
            valueMenu->setTitle(s);

            /* Add a color icon */
            if (ch->group() == QLCChannel::Colour)
                valueMenu->setIcon(colorIcon(cap->name()));

            for (int i = cap->min(); i <= cap->max(); i++)
            {
                action = valueMenu->addAction(
                             t.sprintf("%.3d", i));
                action->setData(i);
            }

            m_menu->addMenu(valueMenu);
        }
        else
        {
            // Just one value in this range, put that into the menu
            action = m_menu->addAction(s);
            action->setData(cap->min());

            /* Add a color icon */
            if (ch->group() == QLCChannel::Colour)
                action->setIcon(colorIcon(cap->name()));
        }
    }

    // Connect menu item activation signal to this
    connect(m_menu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotContextMenuTriggered(QAction*)));

    // Set the menu also as the preset button's popup menu
    m_presetButton->setMenu(m_menu);
}

const QIcon ConsoleChannel::colorIcon(const QString& name)
{
    /* Return immediately with a rainbow icon -- if appropriate */
    if (name.toLower().contains("rainbow") ||
            name.toLower().contains("cw") == true)
    {
        return QIcon(":/rainbow.png");
    }
    else if (name.toLower().contains("cto") == true)
    {
        QColor color(255, 201, 0);
        QPixmap pm(32, 32);
        pm.fill(color);
        return QIcon(pm);
    }
    else if (name.toLower().contains("ctb") == true)
    {
        QColor color(0, 128, 190);
        QPixmap pm(32, 32);
        pm.fill(color);
        return QIcon(pm);
    }
    else if (name.toLower().contains("uv") == true)
    {
        QColor color(37, 0, 136);
        QPixmap pm(32, 32);
        pm.fill(color);
        return QIcon(pm);
    }

#ifdef Q_WS_X11
    QColor::setAllowX11ColorNames(true);
#endif
    QStringList colorList(QColor::colorNames());
    QString colname;
    QColor color;
    int index;

    colname = name.toLower().remove(QRegExp("[0-9]")).remove(' ');
    index = colorList.indexOf(colname);
    if (index != -1)
    {
        color.setNamedColor(colname);
    }
    else
    {
        QString re("(");
        QListIterator <QString> it(name.toLower().split(" "));
        while (it.hasNext() == true)
        {
            re += it.next();
            if (it.hasNext() == true)
                re += "|";
        }
        re += ")";

        QRegExp regex(re, Qt::CaseInsensitive);
        index = colorList.indexOf(regex);
        if (index != -1)
            color.setNamedColor(colorList.at(index));
    }

    if (color.isValid() == true)
    {
        QPixmap pm(32, 32);
        pm.fill(color);
        return QIcon(pm);
    }
    else
    {
        return QIcon();
    }
}

void ConsoleChannel::contextMenuEvent(QContextMenuEvent* e)
{
    // Show the preset menu only of it has been created.
    // Generic dimmer fixtures don't have capabilities and so
    // they will not have these menus either.
    if (m_menu != NULL)
    {
        m_menu->exec(e->globalPos());
        e->accept();
    }
}

void ConsoleChannel::slotContextMenuTriggered(QAction* action)
{
    Q_ASSERT(action != NULL);

    // The menuitem's data contains a valid DMX value
    setValue(action->data().toInt());
}

/*****************************************************************************
 * Value
 *****************************************************************************/

int ConsoleChannel::sliderValue() const
{
    return m_valueSlider->value();
}

void ConsoleChannel::setOutputDMX(bool state)
{
    m_outputDMX = state;
}

void ConsoleChannel::setValue(uchar value)
{
    m_valueSlider->setValue(static_cast<int> (value));
}

void ConsoleChannel::slotValueEdited(const QString& text)
{
    setValue(text.toInt());
}

void ConsoleChannel::slotValueChange(int value)
{
    if (m_value != value)
    {
        m_value = value;
        m_valueEdit->setText(QString("%1").arg(m_value));
        emit valueChanged(m_channel, m_value, isEnabled());

        /* Use a mutex for m_valueChanged so that the latest value
           is really written. */
        m_valueChangedMutex.lock();
        m_valueChanged = true;
        m_valueChangedMutex.unlock();
    }
}

/*****************************************************************************
 * DMXSource
 *****************************************************************************/

void ConsoleChannel::writeDMX(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);

    if (m_outputDMX == false)
        return;

    m_valueChangedMutex.lock();

    const QLCChannel* qlcch = m_fixture->channel(m_channel);
    Q_ASSERT(qlcch != NULL);

    if (qlcch->group() != QLCChannel::Intensity && m_valueChanged == false)
    {
        /* Value has not changed and this is not an intensity channel.
           LTP in effect. */
    }
    else
    {
        quint32 ch = m_fixture->universeAddress() + m_channel;
        universes->write(ch, m_value, qlcch->group());
    }

    m_valueChanged = false;
    m_valueChangedMutex.unlock();
}

/*****************************************************************************
 * Enable/disable
 *****************************************************************************/

void ConsoleChannel::enable(bool state)
{
    setChecked(state);

    const UniverseArray* unis(_app->outputMap()->peekUniverses());
    m_value = unis->preGMValues()[m_fixture->universeAddress() + m_channel];

    emit valueChanged(m_channel, m_value, isEnabled());
}

void ConsoleChannel::slotToggled(bool state)
{
    emit valueChanged(m_channel, m_value, state);
}

/*****************************************************************************
 * Checkable
 *****************************************************************************/

void ConsoleChannel::setCheckable(bool checkable)
{
    if (layout() != NULL)
    {
        if (checkable == true)
        {
            layout()->setContentsMargins(CMARGIN_LEFT,
                                         CMARGIN_TOP_CHECK,
                                         CMARGIN_RIGHT,
                                         CMARGIN_BOTTOM);
        }
        else
        {
            layout()->setContentsMargins(CMARGIN_LEFT,
                                         CMARGIN_TOP,
                                         CMARGIN_RIGHT,
                                         CMARGIN_BOTTOM);
        }
    }

    QGroupBox::setCheckable(checkable);
}
