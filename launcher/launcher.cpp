#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QByteArray>
#include <QPixmap>
#include <QLabel>
#include <unistd.h>

#include "launcher.h"
#include "qlctypes.h"

Launcher::Launcher(char* const* argv, QWidget* parent)
	: QWidget(parent)
	, m_argv(argv)
{
	QGridLayout* lay;
	lay = new QGridLayout(this);
	setLayout(lay);

	QLabel* icon = new QLabel(this);
	icon->setPixmap(QIcon(":/qlc.png").pixmap(64));
	lay->addWidget(icon, 0, 0, 1, 1);

	QString text("<H1>Q Light Controller %1</H1>");
	QLabel* title = new QLabel(text.arg(VERSION), this);
	lay->addWidget(title, 0, 1, 1, 2);

	QPushButton* fxed = new QPushButton(tr("Fixture Editor"), this);
	fxed->setToolTip(tr("Launch fixture definition editor"));
	connect(fxed, SIGNAL(clicked()), this, SLOT(slotFXEDClicked()));
	lay->addWidget(fxed, 1, 1, 1, 1);

	QPushButton* qlc = new QPushButton(tr("Q Light Controller"), this);
	qlc->setToolTip(tr("Launch the main Q Light Controller application"));
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
