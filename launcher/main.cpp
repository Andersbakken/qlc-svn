#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDebug>

#include "qlcconfig.h"
#include "launcher.h"

int main(int argc, char* const* argv)
{
	QApplication app(argc, (char**) argv);

	QString locale(QLocale::system().name());
	QTranslator translator;
	translator.load(QString("launcher_%1").arg(locale),
		QString("%1/../%2").arg(QApplication::applicationDirPath())
				   .arg(TRANSLATIONDIR));
	app.installTranslator(&translator);

	Launcher launcher(argv);
	launcher.show();

	return app.exec();
}
