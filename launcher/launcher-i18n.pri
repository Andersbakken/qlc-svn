# Translation source files
TRANSLATIONS    += launcher_fi_FI.ts

# Translation object files
translations.path   = $$INSTALLROOT/$$TRANSLATIONDIR
translations.files += launcher_fi_FI.qm
INSTALLS           += translations

# Force qmake to create these .qm files
i18n.target = launcher_fi_FI.qm

# Compile all .ts files specified above
unix {
	# Some unices have "lrelease-qt4" while others have "lrelease"
	isEmpty($$system("which lrelease-qt4")) {
		i18n.commands += lrelease launcher-i18n.pri
	} else {
		i18n.commands += lrelease-qt4 launcher-i18n.pri
	}
} else {
	# Windows has "lrelease"
	i18n.commands += lrelease launcher-i18n.pri
}

QMAKE_EXTRA_TARGETS += i18n

PRE_TARGETDEPS += launcher_fi_FI.qm
QMAKE_CLEAN += launcher_fi_FI.qm
QMAKE_DISTCLEAN += launcher_fi_FI.qm

