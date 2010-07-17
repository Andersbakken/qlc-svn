# Translation source files
TRANSLATIONS    += qlc_fi_FI.ts

# Force qmake to create all .qm files
i18n.target = qlc_fi_FI.qm

# Translations installation
translations.path = $$INSTALLROOT/$$TRANSLATIONDIR
translations.files += qlc_fi_FI.qm
INSTALLS        += translations

# Compile all .ts files specified above
unix {
        # Some unices have "lrelease-qt4" while others have "lrelease"
        isEmpty($$system("which lrelease-qt4")) {
                i18n.commands += lrelease src-i18n.pri
        } else {
                i18n.commands += lrelease-qt4 src-i18n.pri
        }
} else {
        # Windows has "lrelease"
        i18n.commands += lrelease src-i18n.pri
}

QMAKE_EXTRA_TARGETS += i18n

PRE_TARGETDEPS += qlc_fi_FI.qm
QMAKE_CLEAN += qlc_fi_FI.qm
QMAKE_DISTCLEAN += qlc_fi_FI.qm
