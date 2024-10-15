qt_add_library(nanoshape STATIC)

find_package(Qt6 REQUIRED COMPONENTS Gui Quick ShaderTools)

target_link_libraries(nanoshape PRIVATE
    Qt6::Quick
    Qt6::GuiPrivate
)

target_include_directories(nanoshape
    PUBLIC include
    PRIVATE nanovg
)

target_compile_definitions(nanoshape PRIVATE
    NVG_NO_STB
    NVG_NO_FONT
    _CRT_SECURE_NO_WARNINGS
)

target_sources(nanoshape PRIVATE
    include/NanoBrush.h
    include/NanoPainter.h
    include/NanoShape.h
    nanovg/nanovg.h
    nanovg/nanovg.c
    src/NanoBrush.cpp
    src/NanoMaterial.cpp
    src/NanoMaterial.h
    src/NanoPainter.cpp
    src/NanoShape.cpp
)

qt_add_shaders(nanoshape "NanoShaders"
    PREFIX /NanoShape
    BASE shaders

    FILES
    shaders/NanoShader.vert
    shaders/NanoShader.frag
)

qt_extract_metatypes(nanoshape)
