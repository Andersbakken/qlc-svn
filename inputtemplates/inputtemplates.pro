include(../variables.pri)

TEMPLATE = subdirs
TARGET = templates

template.files += Behringer-BCF2000.qxi
templates.files += Novation-KS5.qxi

templates.path = $$INPUTTEMPLATEDIR
INSTALLS += templates
