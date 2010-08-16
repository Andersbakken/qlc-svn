# Translation source files
TRANSLATIONS    += fixtureeditor_fi_FI.ts
TRANSLATIONS    += fixtureeditor_fr_FR.ts

# Force qmake to create all .qm files
i18n.path = 
POST_TARGETDEPS += i18n

# Translations installation
translations.path   = $$INSTALLROOT/$$TRANSLATIONDIR
translations.files += fixtureeditor_fi_FI.qm
translations.files += fixtureeditor_fr_FR.qm
INSTALLS           += translations

# Compile all .ts files specified above
unix {
        # Some unices have "lrelease-qt4" while others have "lrelease"
        isEmpty($$system("which lrelease-qt4")) {
                i18n.commands += lrelease fixtureeditor-i18n.pri
        } else {
                i18n.commands += lrelease-qt4 fixtureeditor-i18n.pri
        }
} else {
        # Windows has "lrelease"
        i18n.commands += lrelease fixtureeditor-i18n.pri
}

QMAKE_EXTRA_TARGETS += i18n
QMAKE_CLEAN += fixtureeditor_fi_FI.qm fixtureeditor_fr_FR.qm
QMAKE_DISTCLEAN += fixtureeditor_fi_FI.qm fixtureeditor_fr_FR.qm
