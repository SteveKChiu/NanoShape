INCLUDEPATH += \
    $$PWD/include

LIBS += \
    -L$$shadowed($$PWD) \
    -lnanoshape

win32-msvc*:POST_TARGETDEPS += \
    $$shadowed($$PWD)/nanoshape.lib

else::POST_TARGETDEPS += \
    $$shadowed($$PWD)/libnanoshape.a
