/*
  Q Light Controller
  vcsoloframe.cpp

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

#include <QStyleOptionFrameV2>
#include <QMetaObject>
#include <QMessageBox>
#include <QPainter>
#include <QAction>
#include <QStyle>
#include <QDebug>
#include <QPoint>
#include <QSize>
#include <QMenu>
#include <QList>
#include <QtXml>

#include "qlcfile.h"
#include "virtualconsole.h"
#include "vcsoloframe.h"
#include "vcbutton.h"
#include "function.h"
#include "app.h"
#include "doc.h"

extern App* _app;

VCSoloFrame::VCSoloFrame(QWidget* parent) : VCFrame(parent)
{
    /* Set the class name "VCSoloFrame" as the object name as well */
    setObjectName(VCSoloFrame::staticMetaObject.className());

    m_frameStyle = KVCFrameStyleSunken;    
}

VCSoloFrame::~VCSoloFrame()
{
}


/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCSoloFrame::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCSoloFrame* frame = new VCSoloFrame(parent);
    if (frame->copyFrom(this) == false)
    {
        delete frame;
        frame = NULL;
    }

    return frame;
}

/*****************************************************************************
* Solo behaviour
*****************************************************************************/
void VCSoloFrame::slotModeChanged(Doc::Mode mode)
{	
	VCFrame::slotModeChanged(mode);
	
    // Get all buttons in this soloFrame
    QListIterator <VCButton*> it(findChildren<VCButton*>());
    
    while (it.hasNext() == true)
    {
        VCButton* button = it.next();
        
        // make sure the buttons nearest soloframe is this        
        if (thisIsNearestSoloFrameParent(button))
        {			
            if (mode == Doc::Operate)
            {
                // listen to when the button function is started
                connect(button, SIGNAL(functionStarting()),
                    this, SLOT(slotButtonFunctionStarting()), 
                    Qt::DirectConnection);
            }
            else
            {
                // remove listener
                connect(button, SIGNAL(functionStarting()),
                    this, SLOT(slotButtonFunctionStarting()));
            }
        }
    }    
}

bool VCSoloFrame::thisIsNearestSoloFrameParent(QWidget* widget)
{    
	VCSoloFrame* sf;
	
	while (widget != NULL)
    {
        widget = widget->parentWidget();
        
        sf = qobject_cast<VCSoloFrame*>(widget);
        
        if (sf != NULL)
        {
			return sf == this;
		}
    }
    
    return false;
}

void VCSoloFrame::slotButtonFunctionStarting()
{
    VCButton* senderButton = qobject_cast<VCButton*>(sender());

    if (senderButton != NULL)
    {
        // get every button that is a child of this soloFrame and turn their functions off
        QListIterator <VCButton*> it(findChildren<VCButton*>());
    
        while (it.hasNext() == true)
        {
            VCButton* button = it.next();
            
            if (button->action() == VCButton::Toggle)
            {				
                Function* f = _app->doc()->function(button->function());
                if (f != NULL)
                {
                    f->stopAndWait();
                }
            }
        }
    }
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCSoloFrame::loader(const QDomElement* root, QWidget* parent)
{
    VCSoloFrame* soloframe = NULL;

    Q_ASSERT(root != NULL);
    Q_ASSERT(parent != NULL);

    if (root->tagName() != KXMLQLCVCSoloFrame)
    {
        qDebug() << "Frame node not found!";
        return false;
    }

    /* Create a new frame into its parent */
    soloframe = new VCSoloFrame(parent);
    soloframe->show();

    /* Continue loading */
    return soloframe->loadXML(root);
}

void VCSoloFrame::paintEvent(QPaintEvent* e)
{
	/* Draw a selection frame if appropriate */
    //VCFrame::paintEvent(e);
    
	/* Draw a red dotted line inside the widget */
	/*QPainter painter(this);
	
	QPen pen(Qt::DashLine);
	pen.setCapStyle(Qt::FlatCap);
	pen.setWidth(2);
	pen.setColor(Qt::red);
	painter.setPen(pen);
	//int margin = 0;
	//painter.drawRect(margin, margin, rect().width() - margin*2, rect().height() - margin*2);	
	painter.drawRect(0, 0, rect().width() - 1, rect().height() - 1);	
	painter.end();*/
		
    /* No point coming here if there is no VC instance */
    VirtualConsole* vc = VirtualConsole::instance();
    if (vc == NULL)
        return;
        
    QPainter painter(this);

    /* Draw frame according to style */
    /*QStyleOptionFrame option;
    option.initFrom(this);
        
    if (frameStyle() == KVCFrameStyleSunken)
        option.state = QStyle::State_Sunken;
    else if (frameStyle() == KVCFrameStyleRaised)
        option.state = QStyle::State_Raised;
    else
        option.state = QStyle::State_None;
*/
    /* Draw a frame border if such is specified for this widget */
  /*  if (option.state != QStyle::State_None)
    {
        style()->drawPrimitive(QStyle::PE_Frame, &option,
                               &painter, this);
    }*/

    QWidget::paintEvent(e);

    /* Draw selection frame */
    bool drawSelectionFrame = mode() == Doc::Design && vc->isWidgetSelected(this);

    /* Draw a dotted line around the widget */
    QPen pen(drawSelectionFrame ? Qt::DotLine : Qt::DashLine);
    pen.setColor(Qt::red);
    
    if (drawSelectionFrame)
    {
        pen.setCapStyle(Qt::RoundCap);
        pen.setWidth(0);        
    }
    else
    {     
        pen.setCapStyle(Qt::FlatCap);
        pen.setWidth(1);        
    }
    
    painter.setPen(pen);
    painter.drawRect(0, 0, rect().width()-1, rect().height()-1);
        
    if (drawSelectionFrame)
    {
        /* Draw a resize handle */
        QIcon icon(":/resize.png");
        painter.drawPixmap(rect().width() - 16, rect().height() - 16,
                            icon.pixmap(QSize(16, 16), QIcon::Normal, QIcon::On));
    }
}
