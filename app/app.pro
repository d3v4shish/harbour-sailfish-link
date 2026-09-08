TARGET = harbour-sailfish-link
TEMPLATE = app

CONFIG += sailfishapp c++11
QT += core gui qml quick network

INCLUDEPATH += ../src/common

SOURCES += \
    src/main.cpp \
    src/linkmodel.cpp \
    ../src/common/linkcommon.cpp

HEADERS += \
    src/linkmodel.h \
    ../src/common/linkcommon.h

DISTFILES += \
    qml/harbour-sailfish-link.qml \
    qml/pages/MainPage.qml \
    qml/pages/FirstRunPage.qml \
    qml/cover/CoverPage.qml

appbin.files = harbour-sailfish-link
appbin.path = /usr/bin

qmlfiles.files = qml
qmlfiles.path = /usr/share/harbour-sailfish-link

desktopfile.files = ../desktop/harbour-sailfish-link.desktop
desktopfile.path = /usr/share/applications

iconfile.files = ../icons/86x86/harbour-sailfish-link.png
iconfile.path = /usr/share/icons/hicolor/86x86/apps

INSTALLS += appbin qmlfiles desktopfile iconfile
