#include <QApplication>
#include "launcher.h"

int main(int argc, char* const* argv)
{
	QApplication app(argc, (char**) argv);

	Launcher launcher(argv);
	launcher.show();

	return app.exec();
}
