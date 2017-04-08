#-------------------------------------------------
#
# Project created by QtCreator 2017-02-25T13:16:36
#
#-------------------------------------------------

QT       -= core gui

TARGET = OpenHantek2xxx-5xxx
TEMPLATE = lib
CONFIG += staticlib
QMAKE_CXXFLAGS += -std=c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += hantekDeviceAcquisition.cpp  hantekDevice.cpp  hantekDeviceInit.cpp  init.cpp  protocol.cpp

HEADERS += hantekDevice.h  init.h  protocolBulk.h  protocolControl.h  protocol.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += ../libusbDSO
