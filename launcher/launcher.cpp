#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QByteArray>
#include <QPixmap>
#include <QLabel>
#include <unistd.h>

#include "qlcconfig.h"
#include "qlctypes.h"
#include "launcher.h"

Launcher::Launcher(char* const* argv, QWidget* parent)
	: QWidget(parent)
	, m_argv(argv)
{
	QGridLayout* lay;
	lay = new QGridLayout(this);
	setLayout(lay);

	setWindowTitle(APPNAME);

	QLabel* icon = new QLabel(this);
	icon->setPixmap(QIcon(":/qlc.png").pixmap(64));
	lay->addWidget(icon, 0, 0, 1, 1);

	QString text("<H1>%1 %2</H1>");
	QLabel* title = new QLabel(text.arg(APPNAME).arg(APPVERSION), this);
	lay->addWidget(title, 0, 1, 1, 2);

	QPushButton* fxed = new QPushButton(FXEDNAME, this);
	fxed->setToolTip(tr("Launch %1").arg(FXEDNAME));
	connect(fxed, SIGNAL(clicked()), this, SLOT(slotFXEDClicked()));
	lay->addWidget(fxed, 1, 1, 1, 1);

	QPushButton* qlc = new QPushButton(APPNAME, this);
	qlc->setToolTip(tr("Launch the main %1 application").arg(APPNAME));
	connect(qlc, SIGNAL(clicked()), this, SLOT(slotQLCClicked()));
	lay->addWidget(qlc, 1, 2, 1, 1);
}

Launcher::~Launcher()
{
}

void Launcher::slotFXEDClicked()
{
	QString path(QApplication::applicationDirPath());
	if (path.endsWith(QString("/")) == false)
		path += QString("/");
	path += QString("qlc-fixtureeditor");
	::execv(path.toAscii().constData(), m_argv);
	QApplication::exit();
}

void Launcher::slotQLCClicked()
{
	QString path(QApplication::applicationDirPath());
	if (path.endsWith(QString("/")) == false)
		path += QString("/");
	path += QString("qlc");
	::execv(path.toAscii().constData(), m_argv);
	QApplication::exit();
}
