TEMPLATE = lib
CONFIG += staticlib c++17
CONFIG -= debug_and_release
QT += quick

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
    shaders/NanoLegacyShader.vert \
    shaders/NanoLegacyShader.frag \
    shaders/NanoShader.vert \
    shaders/NanoShader.frag

versionAtLeast(QT_VERSION, 6) {
    RESOURCES += \
        shaders/NanoShaders.qrc
}
else:versionAtLeast(QT_VERSION, 5.15) {
    RESOURCES += \
        shaders/NanoShaders.qrc \
        shaders/NanoLegacyShaders.qrc \
}
else {
    RESOURCES += \
        shaders/NanoLegacyShaders.qrc \
}

