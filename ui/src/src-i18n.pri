# Translation source files
TRANSLATIONS    += qlc_fi_FI.ts
TRANSLATIONS    += qlc_fr_FR.ts

# Force qmake to create all .qm files
i18n.path = 
PRE_TARGETDEPS += i18n

# Translations installation
translations.path   = $$INSTALLROOT/$$TRANSLATIONDIR
translations.files += qlc_fi_FI.qm
translations.files += qlc_fr_FR.qm
INSTALLS           += translations

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
QMAKE_CLEAN += qlc_fi_FI.qm qlc_fr_FR.qm
QMAKE_DISTCLEAN += qlc_fi_FI.qm qlc_fr_FR.qm
