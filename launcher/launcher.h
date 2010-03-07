#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QWidget>

class Launcher : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(Launcher)

public:
	Launcher(char* const* argv, QWidget* parent = 0);
	~Launcher();

private slots:
	void slotFXEDClicked();
	void slotQLCClicked();

private:
	char* const* m_argv;
};

#endif
