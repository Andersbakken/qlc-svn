#include <QDBusConnection>
#include <QDebug>

#include "mockutil.h"

bool QDBusConnection::connect(const QString& service, const QString& path,
                              const QString& interface, const QString& name,
                              QObject* receiver, const char* slot)
{
    UT_ASSERT(service == QString());
    UT_ASSERT(path == QString("/org/freedesktop/Hal/Manager"));
    UT_ASSERT(interface == QString("org.freedesktop.Hal.Manager"));
    UT_ASSERT(name == QString("DeviceAdded") || name == QString("DeviceRemoved"));
    UT_ASSERT(receiver != NULL);
    UT_ASSERT(QString(slot) == QString(SLOT(slotDeviceAdded(const QString&))) ||
              QString(slot) == QString(SLOT(slotDeviceRemoved(const QString&))));

    return true;
}
