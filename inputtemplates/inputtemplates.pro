TEMPLATE = subdirs
TARGET = templates

templates.files += Novation-KS5.qxi

unix:templates.path = /usr/share/inputtemplates
win32:templates.path = C:/QLC/InputTemplates
INSTALLS += templates
