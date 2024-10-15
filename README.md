# NanoShape

NanoShape is a C++ vector drawing library for QtQuick scene graph, offering great performance and rendering quality. 

NanoShape is designed to work with QtQuick, either in C++ or qml directly.

* Use NanoShape in qml directly, it is more or less like the qml `Canvas`, but is completely hardware accelerated
  and without indirect frame buffer object requirement.

* Use NanoPainter in your `QQuickItem` class, make it easy to write `updatePaintNode` method.

NanoShape uses [NanoVG](https://github.com/memononen/nanovg) as its rendering backend.
And some modification has been made to NanoVG to support drawing dash line directly.

## Features

* Works with `QQuickItem` C++ class.
* Works with qml directly.
* Works with Qt 5.x (opengl) or Qt 6 (RHI)
* Works with RHI, so it runs natively in opengl, vulkan, direct3d11, metal or other RHI backend.
* Path-based drawing of various shape, rectangles, circles, lines etc, filled and stroked. 
* Brush can be color, gradient, image pattern or dash pattern. 
* Line cap and join options.
* Dash line pattern options.
* Antialiasing can be turn on or off based on Item.antialiasing property.

## Setup for qmake

* Copy `nanoshape` sub directory into your project. (e.g. `ext/nanoshape`)

* The `nanoshape.pro` is static lib that you can build and link with your code, 
  you should have a subdirs `project.pro` in your project like the following:

```
TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += ext/nanoshape/nanoshape.pro
SUBDIRS += app.pro
```

* Then in your `app.pro`, include the `nanoshape.pri` file, path need to match your project structure.

```
include(ext/nanoshape/nanoshape.pri)
```

## Setup for cmake

* Copy `nanoshape` sub directory into your project. (e.g. `ext/nanoshape`)

* using `add_subdirectory(ext/nanoshape)` in your top level CMakeLists.txt

* using `target_link_libraries` to add `nanoshape` to your project

## Use NanoShape in QML

For convenience, the `NanoShape` class is provided to be used in qml directly.
Since this lib is hybrid qml/c++ lib and not a qml module, you have to register the `NanoShape` class before
you can use it in qml:

```c++
// the old Qt5 qml registration, in your cpp file
#include "NanoShape.h"

void registerQmlTypes()
{
    // register the nanoshape types
    qmlRegisterType<NanoShape>("NanoShape", 1, 0, "NanoShape");
    qmlRegisterUncreatableType<NanoShapePainter>("NanoShape", 1, 0, "NanoShapePainter", "inner class of NanoShape");
}
```

In Qt6 with cmake, it would be better to import as foreign types.

```c++
// the new Qt6 qml registration, in file NanoShapeForeignTypes.h
#pragma once

#include "NanoShape.h"

class Import_NanoShape
{
    Q_GADGET
    QML_NAMED_ELEMENT(NanoShape)
    QML_FOREIGN(NanoShape)
};

class Import_NanoShapePainter
{
    Q_GADGET
    QML_NAMED_ELEMENT(NanoShapePainter)
    QML_FOREIGN(NanoShapePainter)
    QML_UNCREATABLE("internal class")
};

```

Once it is done you can start using it in qml:

```
import NanoShape 1.0

NanoShape {
    id: _shape
    width: 500
    height: 500

    property color strokeColor: "darkBlue"
    property real strokeWidth: 5

    onStrokeColorChanged: _shape.markDirty()
    onStrokeWidthChanged: _shape.markDirty()

    onPaint: (painter) => {
        painter.moveTo(10, 10)
        painter.bezierTo(100, 200, 100, 300, _shape.width, _shape.height)
        painter.setStrokeWidth(_shape.strokeWidth)
        painter.setDashPattern([4, 1, 1, 1])
        painter.setStrokeStyle(_shape.strokeColor)
        painter.stroke()
    }
}
```

Unlike the Canvas item, NanoShape does not require frame buffer object and is completely hardware accelerated.

## Use NanoShape in C++

For even better performance, you may want to write your own QQuickItem class.
The `NanoPainter` make it very easy to write the `updatePaintNode` method:

```c++
#include "NanoPainter.h"

MyItem::MyItem(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
    setAntialiasing(true);
}

QSGNode* MyItem::updatePaintNode(QSGNode* node, QQuickItem::UpdatePaintNodeData*)
{
    NanoPainter painter(this, node);
    painter.beginPath("shape"); // this is optional
    painter.moveTo(10, 10);
    painter.bezierTo(100, 200, 100, 300, width(), height());
    painter.setStrokeWidth(m_strokeWidth);
    painter.setDashPattern({4.0, 1.0, 1.0, 1.0});
    painter.setStrokeBrush(m_strokeColor);
    painter.stroke();
    return painter.updatePaintNode();
}

```

## Advanced usage in C++

For more advance usage, you can avoid constructing the `QSGNode` every time.
For example if you only need to change the color, it should be possible to update 
the material without constructing the whole QSGNode again.

In the last example, what if we want to change the color only, is there a better way?

```c++
QSGNode* MyItem::updatePaintNode(QSGNode* node, QQuickItem::UpdatePaintNodeData*)
{
    // Fast path to update the brush only.
    // Only brush can be updated directly, others like stroke-width change will require
    // constructing the QSGNode again.
    // So if you update the m_strokeWidth, you should set m_nodeDirty to true.
    if (node && !m_nodeDirty) {
        auto ok = NanoPainter::updatePaintNodeStrokeBrush(this, node, "shape", m_strokeColor);
        if (ok) return node;
    }

    NanoPainter painter(this, node);
    painter.beginPath("shape");
    painter.moveTo(10, 10);
    painter.bezierTo(100, 200, 100, 300, width(), height());
    painter.setStrokeWidth(m_strokeWidth);
    painter.setDashPattern({4.0, 1.0, 1.0, 1.0});
    painter.setStrokeBrush(m_strokeColor);
    painter.stroke();
    return painter.updatePaintNode();
}

```

Please see `NanoShapeExample.cpp` for more completed example.

## Links

* [Qt](https://www.qt.io)
* [NanoVG](https://github.com/memononen/nanovg)
* [NanoShape](https://github.com/SteveKChiu/NanoShape)
