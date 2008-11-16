include(../variables.pri)

TEMPLATE = subdirs
TARGET = templates

templates.files += Novation-KS5.qxi

templates.path = $$INPUTTEMPLATEDIR
INSTALLS += templates
