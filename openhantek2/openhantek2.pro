QT += qml quick widgets printsupport

QMAKE_CXXFLAGS += -std=c++11

VERSION = "0.0"

HEADERS += \
    src/currentdevice.h \
    src/devicemodel.h \
    src/exporter.h \
    src/modeldatasetter.h \
    src/parameters.h \
    src/scopecolors.h \
    src/strings/dsoerrorstrings.h \
    src/strings/dsostrings.h \
    src/strings/unitToString.h \
    plot/qpcurve.h \
    plot/qpfixedscaleengine.h \
    plot/qpplotitem.h \
    plot/qpscaleengine.h \
    plot/qpscrollingcurve.h \
    plot/qptightscaleengine.h

SOURCES += \
    src/main.cpp \
    src/currentdevice.cpp \
    src/devicemodel.cpp \
    src/exporter.cpp \
    src/modeldatasetter.cpp \
    src/parameters.cpp \
    src/scopecolors.cpp \
    src/strings/dsoerrorstrings.cpp \
    src/strings/dsostrings.cpp \
    src/strings/unitToString.cpp \
    plot/qpcurve.cpp \
    plot/qpfixedscaleengine.cpp \
    plot/qpplotitem.cpp \
    plot/qpscaleengine.cpp \
    plot/qpscrollingcurve.cpp \
    plot/qptightscaleengine.cpp


LIBS += -L../libDemoDevice -L../libOpenHantek2xxx-5xxx -L../libOpenHantek60xx -L../libPostprocessingDSO -L../libusbDSO \
        -lDemoDevice -lOpenHantek2xxx-5xxx -lOpenHantek60xx -lPostprocessingDSO -lusbDSO -lusb-1.0 -lfftw3
RESOURCES += ui/qml.qrc

INCLUDEPATH += . .. ../libusbDSO ../libPostprocessingDSO

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH = ui/QuickPlot

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
