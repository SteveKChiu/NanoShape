TEMPLATE = app
CONFIG += c++17
CONFIG -= debug_and_release
QT += quick

include(../nanoshape/nanoshape.pri)

HEADERS += \
    NanoShapeExample.h

SOURCES += \
    NanoShapeExample.cpp \
    main.cpp

DISTFILES += \
    main.qml

RESOURCES += \
    $$DISTFILES
    