#include <QtCore>
#include <QtGui>

#include "enttecdmxusbout.h"
#include "enttecdmxusbwidget.h"

int main(int argc, char** argv)
{
	QApplication qapp(argc, argv);

	EnttecDMXUSBOut dmxusb;

	int r;
	do {
		dmxusb.rescanWidgets();

		QStringListIterator it(dmxusb.outputs());
		while (it.hasNext() == true)
		{
			QString widget = it.next();
			qDebug() << widget;
		}

		dmxusb.open(0);

		QByteArray ba(512, 0);
		dmxusb.writeRange(0, 0, (t_value*) ba.data(), 512);

		r = QMessageBox::question(NULL, "Rescan?",
			"Rescan available Enttec DMX USB widgets?",
			QMessageBox::Yes, QMessageBox::No);

		dmxusb.close(0);

	} while (r == QMessageBox::Yes);

	return 0;
}
