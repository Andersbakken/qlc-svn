/****************************************************************************
** UI_ConfigureLlaOut meta object code from reading C++ file 'uic_configurellaout.h'
**
** Created: Tue Apr 10 18:50:28 2007
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "uic_configurellaout.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *UI_ConfigureLlaOut::className() const
{
    return "UI_ConfigureLlaOut";
}

QMetaObject *UI_ConfigureLlaOut::metaObj = 0;
static QMetaObjectCleanUp cleanUp_UI_ConfigureLlaOut( "UI_ConfigureLlaOut", &UI_ConfigureLlaOut::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString UI_ConfigureLlaOut::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "UI_ConfigureLlaOut", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString UI_ConfigureLlaOut::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "UI_ConfigureLlaOut", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* UI_ConfigureLlaOut::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"slotActivateClicked", 0, 0 };
    static const QUMethod slot_1 = {"languageChange", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "slotActivateClicked()", &slot_0, QMetaData::Public },
	{ "languageChange()", &slot_1, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"UI_ConfigureLlaOut", parentObject,
	slot_tbl, 2,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_UI_ConfigureLlaOut.setMetaObject( metaObj );
    return metaObj;
}

void* UI_ConfigureLlaOut::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "UI_ConfigureLlaOut" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool UI_ConfigureLlaOut::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotActivateClicked(); break;
    case 1: languageChange(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool UI_ConfigureLlaOut::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool UI_ConfigureLlaOut::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool UI_ConfigureLlaOut::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
