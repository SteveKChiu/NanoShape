TEMPLATE = lib
CONFIG += staticlib c++17
CONFIG -= debug_and_release
QT += quick
TARGET = $$qt5LibraryTarget(nanoshape)

DEFINES += \
    NVG_NO_STB \
    NVG_NO_FONT \
    _CRT_SECURE_NO_WARNINGS

INCLUDEPATH += \
    nanovg \
    include

HEADERS += \
    include/NanoBrush.h \
    include/NanoPainter.h \
    include/NanoShape.h \
    nanovg/nanovg.h \
    src/NanoMaterial.h

SOURCES += \
    nanovg/nanovg.c \
    src/NanoBrush.cpp \
    src/NanoMaterial.cpp \
    src/NanoPainter.cpp \
    src/NanoShape.cpp

DISTFILES += \
    shaders/NanoShaderGLES.vert \
    shaders/NanoShaderGLES.frag

RESOURCES += \
    shaders/NanoShadersGLES.qrc
