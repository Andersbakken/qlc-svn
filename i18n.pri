# Compile all .ts files specified above
qmfiles.path = $$INSTALLROOT/$$TRANSLATIONDIR
unix {
        # Some unices have "lrelease-qt4" while others have "lrelease"
        isEmpty($$system("which lrelease-qt4")) {
                qmfiles.commands += lrelease $$PRO_FILE
        } else {
                qmfiles.commands += lrelease-qt4 $$PRO_FILE
        }
} else {
        # Windows has "lrelease"
        qmfiles.commands += lrelease $$PRO_FILE
}
POST_TARGETDEPS     += qmfiles
QMAKE_EXTRA_TARGETS += qmfiles

# Translations installation
i18n.path = $$INSTALLROOT/$$TRANSLATIONDIR
for(tsfile, TRANSLATIONS) {
    !isEmpty(i18n.commands):i18n.commands += "&&"
    # Weird hack for linux since "isEmpty($$(INSTALL_ROOT))" won't work
    IROOT=$$(INSTALL_ROOT)
    isEmpty(IROOT) {
        # Install normally
        unix:i18n.commands += $$QMAKE_COPY $$replace(tsfile, ts, qm) $$i18n.path
        win32:i18n.commands += $$QMAKE_COPY $$replace(tsfile, ts, qm) $$replace(i18n.path, /, \\)
    } else {
        # Install to a Debian tmp directory
        i18n.commands += $$QMAKE_COPY $$replace(tsfile, ts, qm) $$(INSTALL_ROOT)/$$i18n.path
    }
}
INSTALLS += i18n

QMAKE_CLEAN     += $$QMFILES
QMAKE_DISTCLEAN += $$QMFILES
