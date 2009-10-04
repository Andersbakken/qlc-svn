#include <QtCore>

#include "enttecdmxusbproout.h"

int main(int argc, char** argv)
{
	QCoreApplication qapp(argc, argv);

	EnttecDMXUSBProOut pro;
	pro.init();

	return qapp.exec();
}
