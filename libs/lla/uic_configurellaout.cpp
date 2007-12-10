/****************************************************************************
** Form implementation generated from reading ui file 'configurellaout.ui'
**
** Created: Mon Dec 10 13:23:31 2007
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "uic_configurellaout.h"

#include <qvariant.h>
#include <qheader.h>
#include <qlistview.h>
#include <qpushbutton.h>
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

    m_listView = new QListView( this, "m_listView" );
    m_listView->addColumn( tr( "Output" ) );
    m_listView->addColumn( tr( "Device" ) );
    m_listView->setResizeMode( QListView::LastColumn );
    UI_ConfigureLlaOutLayout->addWidget( m_listView );

    layout2 = new QHBoxLayout( 0, 0, 6, "layout2"); 

    m_testButton = new QPushButton( this, "m_testButton" );
    layout2->addWidget( m_testButton );

    m_refreshButton = new QPushButton( this, "m_refreshButton" );
    layout2->addWidget( m_refreshButton );
    spacer2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem( spacer2 );

    m_ok = new QPushButton( this, "m_ok" );
    m_ok->setDefault( TRUE );
    layout2->addWidget( m_ok );

    m_cancel = new QPushButton( this, "m_cancel" );
    layout2->addWidget( m_cancel );
    UI_ConfigureLlaOutLayout->addLayout( layout2 );
    languageChange();
    resize( QSize(416, 284).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( m_cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( m_ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( m_testButton, SIGNAL( toggled(bool) ), this, SLOT( slotTestToggled(bool) ) );
    connect( m_refreshButton, SIGNAL( clicked() ), this, SLOT( slotRefreshClicked() ) );

    // tab order
    setTabOrder( m_cancel, m_ok );
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
    m_listView->header()->setLabel( 0, tr( "Output" ) );
    m_listView->header()->setLabel( 1, tr( "Device" ) );
    m_testButton->setText( tr( "Test output" ) );
    m_refreshButton->setText( tr( "Refresh" ) );
    m_ok->setText( tr( "&OK" ) );
    m_ok->setAccel( QKeySequence( tr( "Alt+O" ) ) );
    QToolTip::add( m_ok, tr( "Accept changes and close the dialog" ) );
    m_cancel->setText( tr( "&Cancel" ) );
    m_cancel->setAccel( QKeySequence( tr( "Alt+C" ) ) );
    QToolTip::add( m_cancel, tr( "Reject changes and close the dialog" ) );
}

void UI_ConfigureLlaOut::slotTestToggled(bool)
{
    qWarning( "UI_ConfigureLlaOut::slotTestToggled(bool): Not implemented yet" );
}

void UI_ConfigureLlaOut::slotRefreshClicked()
{
    qWarning( "UI_ConfigureLlaOut::slotRefreshClicked(): Not implemented yet" );
}

