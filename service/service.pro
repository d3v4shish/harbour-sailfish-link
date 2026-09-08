TARGET = harbour-sailfish-link-discovery
TEMPLATE = app

CONFIG += console c++11
CONFIG -= app_bundle
QT += core network

INCLUDEPATH += ../src/common

SOURCES += \
    src/main.cpp \
    ../src/common/linkcommon.cpp

HEADERS += \
    ../src/common/linkcommon.h

appbin.files = harbour-sailfish-link-discovery
appbin.path = /usr/bin

systemdfile.files = harbour-sailfish-link.service
systemdfile.path = /usr/lib/systemd/user

INSTALLS += appbin systemdfile
