INCLUDEPATH += \
    $$PWD/include

LIBS += \
    -L$$shadowed($$PWD) \
    -l$$qt5LibraryTarget(nanoshape)

win32-msvc*:POST_TARGETDEPS += \
    $$shadowed($$PWD)/$$qt5LibraryTarget(nanoshape).lib

else::POST_TARGETDEPS += \
    $$shadowed($$PWD)/lib$$qt5LibraryTarget(nanoshape).a
