#include <QtCore>

#include "enttecdmxusbproout.h"

int main(int argc, char** argv)
{
	QCoreApplication qapp(argc, argv);

	EnttecDMXUSBProOut pro;
	pro.init();

	qDebug() << pro.outputs();

	unsigned char uni[512] = { 0 };
	uni[0] = 64;
	uni[1] = 127;
	uni[2] = 192;
	uni[3] = 255;
	if (pro.outputs().size() > 0)
	{
		pro.open(0);
		pro.writeRange(0, 0, (t_value*) uni, 512);
		pro.close(0);
	}

	return qapp.exec();
}
