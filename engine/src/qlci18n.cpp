/*
  Q Light Controller
  qlci18n.cpp

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

#include <QCoreApplication>
#include <QTranslator>
#include <QLocale>
#include <QString>

#include "qlcconfig.h"
#include "qlci18n.h"

QString QLCi18n::s_defaultLocale = QString();

void QLCi18n::setDefaultLocale(const QString& locale)
{
    s_defaultLocale = locale;
}

QString QLCi18n::defaultLocale()
{
    return s_defaultLocale;
}

void QLCi18n::loadTranslation(const QString& component)
{
    QString lc;

    if (defaultLocale().isEmpty() == true)
        lc = QLocale::system().name();
    else
        lc = defaultLocale();

    QString file(QString("%1_%2").arg(component).arg(lc));

#ifdef __APPLE__
    QString path(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                 .arg(TRANSLATIONDIR));
#else
    QString path(TRANSLATIONDIR);
#endif
    QTranslator* translator = new QTranslator(QCoreApplication::instance());
    if (translator->load(file, path) == true)
        QCoreApplication::installTranslator(translator);
}
