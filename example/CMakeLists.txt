qt_add_executable(NanoShapeExample)

target_sources(NanoShapeExample PRIVATE
    main.cpp
)

qt_add_qml_module(mod_NanoShapeExample
    URI NanoShapeExample
    VERSION 1.0
    RESOURCE_PREFIX "/qml"
    OUTPUT_DIRECTORY "qml/NanoShapeExample"

    SOURCES
    NanoShapeExample.h
    NanoShapeExample.cpp
    NanoShapeForeignTypes.h

    QML_FILES
    main.qml
)

target_link_libraries(mod_NanoShapeExample PRIVATE
    nanoshape
    Qt6::Quick
)

target_link_libraries(NanoShapeExample PRIVATE
    mod_NanoShapeExample
    Qt6::Quick
)
