#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDebug>

#include "qlcconfig.h"
#include "launcher.h"

void loadTranslation(const QString& locale, QApplication& app)
{
	QString file(QString("launcher_%1").arg(locale));
#ifdef __APPLE__
	QString path(QString("%1/../%2").arg(QApplication::applicationDirPath())
					.arg(TRANSLATIONDIR));
#else
	QString path(TRANSLATIONDIR);
#endif
	QTranslator* translator = new QTranslator(&app);
	if (translator->load(file, path) == true)
	{
		qDebug() << "Using translation for" << locale;
		app.installTranslator(translator);
	}
	else
	{
		qDebug() << "Unable to find translation for" << locale
			 << "in" << path;
	}
}

int main(int argc, char* const* argv)
{
	QApplication app(argc, (char**) argv);

	loadTranslation(QLocale::system().name(), app);

	Launcher launcher(argv);
	launcher.show();

	return app.exec();
}
