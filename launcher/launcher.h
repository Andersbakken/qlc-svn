#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QWidget>

/**
 * Launcher is a very simple for displaying an initial launch dialog
 * for choosing, whether to execute QLC main application or Fixture Editor.
 * Once the choice has been made, the launcher window terminates and the
 * chosen application will be started. This kind of launcher is needed for
 * MacOS X because application bundles can launch only one binary program.
 */
class Launcher : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(Launcher)

public:
	/**
	 * Create a new launcher widget, with the given list of
	 * command line arguments and parent.
	 *
	 * @param argv List of command line arguments (must not be deleted)
	 * @param parent Owning parent widget (optional)
	 */
	Launcher(char* const* argv, QWidget* parent = 0);

	/** Destructor */
	~Launcher();

private slots:
	/** Slot for fixture editor button click */
	void slotFXEDClicked();

	/** Slot for main application button click */
	void slotQLCClicked();

private:
	/** Command line arguments */
	char* const* m_argv;
};

#endif
