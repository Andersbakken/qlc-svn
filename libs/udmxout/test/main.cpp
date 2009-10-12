#include <QtCore>

#include "udmxout.h"

int main(int argc, char** argv)
{
	QCoreApplication qapp(argc, argv);

	UDMXOut ud;
	ud.init();

	if (ud.outputs().size() > 0)
	{
		qDebug() << "uDMX devices:" << ud.outputs();

		unsigned char uni[512] = { 0 };
		uni[0] = 64;
		uni[1] = 127;
		uni[2] = 192;
		uni[3] = 255;

		ud.open(0);
		ud.writeRange(0, 0, (t_value*) uni, 512);
		ud.close(0);
	}
	else
	{
		qDebug() << "No uDNX devices present.";
	}

	return 0;
}
