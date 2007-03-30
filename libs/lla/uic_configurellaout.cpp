/****************************************************************************
** Form implementation generated from reading ui file 'configurellaout.ui'
**
** Created: Mon Jan 8 19:21:03 2007
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "uic_configurellaout.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a UI_ConfigureLlaOut as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
UI_ConfigureLlaOut::UI_ConfigureLlaOut( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "UI_ConfigureLlaOut" );
    UI_ConfigureLlaOutLayout = new QVBoxLayout( this, 11, 6, "UI_ConfigureLlaOutLayout"); 

    m_deviceGroup = new QGroupBox( this, "m_deviceGroup" );
    m_deviceGroup->setFrameShape( QGroupBox::Box );
    m_deviceGroup->setFrameShadow( QGroupBox::Sunken );
    m_deviceGroup->setColumnLayout(0, Qt::Vertical );
    m_deviceGroup->layout()->setSpacing( 6 );
    m_deviceGroup->layout()->setMargin( 11 );
    m_deviceGroupLayout = new QGridLayout( m_deviceGroup->layout() );
    m_deviceGroupLayout->setAlignment( Qt::AlignTop );

    m_firstNumberSpinBox = new QSpinBox( m_deviceGroup, "m_firstNumberSpinBox" );
    m_firstNumberSpinBox->setMaxValue( 255 );
    m_firstNumberSpinBox->setMinValue( 0 );
    m_firstNumberSpinBox->setValue( 1 );

    m_deviceGroupLayout->addWidget( m_firstNumberSpinBox, 1, 1 );

    textLabel1_2 = new QLabel( m_deviceGroup, "textLabel1_2" );

    m_deviceGroupLayout->addWidget( textLabel1_2, 1, 0 );
    UI_ConfigureLlaOutLayout->addWidget( m_deviceGroup );

    m_statusGroup = new QGroupBox( this, "m_statusGroup" );
    m_statusGroup->setColumnLayout(0, Qt::Vertical );
    m_statusGroup->layout()->setSpacing( 6 );
    m_statusGroup->layout()->setMargin( 11 );
    m_statusGroupLayout = new QHBoxLayout( m_statusGroup->layout() );
    m_statusGroupLayout->setAlignment( Qt::AlignTop );

    m_statusLabel = new QLabel( m_statusGroup, "m_statusLabel" );
    QFont m_statusLabel_font(  m_statusLabel->font() );
    m_statusLabel_font.setBold( TRUE );
    m_statusLabel->setFont( m_statusLabel_font ); 
    m_statusLabel->setFrameShape( QLabel::StyledPanel );
    m_statusLabel->setFrameShadow( QLabel::Sunken );
    m_statusLabel->setAlignment( int( QLabel::AlignCenter ) );
    m_statusGroupLayout->addWidget( m_statusLabel );

    m_activate = new QPushButton( m_statusGroup, "m_activate" );
    m_statusGroupLayout->addWidget( m_activate );
    UI_ConfigureLlaOutLayout->addWidget( m_statusGroup );

    layout6 = new QHBoxLayout( 0, 0, 6, "layout6"); 
    spacer2 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout6->addItem( spacer2 );

    m_ok = new QPushButton( this, "m_ok" );
    m_ok->setDefault( TRUE );
    layout6->addWidget( m_ok );

    m_cancel = new QPushButton( this, "m_cancel" );
    layout6->addWidget( m_cancel );
    UI_ConfigureLlaOutLayout->addLayout( layout6 );
    languageChange();
    resize( QSize(255, 198).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( m_cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( m_ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( m_activate, SIGNAL( clicked() ), this, SLOT( slotActivateClicked() ) );

    // tab order
    setTabOrder( m_cancel, m_activate );
    setTabOrder( m_activate, m_ok );
}

/*
 *  Destroys the object and frees any allocated resources
 */
UI_ConfigureLlaOut::~UI_ConfigureLlaOut()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void UI_ConfigureLlaOut::languageChange()
{
    setCaption( tr( "Configure LLA Plugin" ) );
    m_deviceGroup->setTitle( tr( "Device" ) );
    QToolTip::add( m_deviceGroup, QString::null );
    QToolTip::add( m_firstNumberSpinBox, tr( "The first universe to send on" ) );
    textLabel1_2->setText( tr( "First Universe" ) );
    m_statusGroup->setTitle( tr( "Status" ) );
    m_statusLabel->setText( tr( "Not Active" ) );
    QToolTip::add( m_statusLabel, tr( "Current interface status" ) );
    m_activate->setText( tr( "&Activate" ) );
    m_activate->setAccel( QKeySequence( tr( "Alt+A" ) ) );
    QToolTip::add( m_activate, tr( "Activate the interface. Pressing this button also saves settings." ) );
    m_ok->setText( tr( "&OK" ) );
    m_ok->setAccel( QKeySequence( tr( "Alt+O" ) ) );
    QToolTip::add( m_ok, tr( "Accept changes and close the dialog" ) );
    m_cancel->setText( tr( "&Cancel" ) );
    m_cancel->setAccel( QKeySequence( tr( "Alt+C" ) ) );
    QToolTip::add( m_cancel, tr( "Reject changes and close the dialog" ) );
}

void UI_ConfigureLlaOut::slotActivateClicked()
{
    qWarning( "UI_ConfigureLlaOut::slotActivateClicked(): Not implemented yet" );
}

