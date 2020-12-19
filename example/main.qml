import QtQuick 2.12
import QtQuick.Controls 2.12

import NanoShape 1.0
import NanoShapeExample 1.0

ApplicationWindow {
    id: _self
    color: "white"
    visibility: ApplicationWindow.Maximized

    // the c++ example
    NanoShapeExample {
        x: 100
        y: 200
        width: 500
        height: 500
        edges: 5
        strokeWidth: 15

        ColorAnimation on fillColor {
            from: "yellow"
            to: "red"
            duration: 5000
            loops: Animation.Infinite
        }

        ColorAnimation on strokeColor {
            from: "blue"
            to: "red"
            duration: 5000
            loops: Animation.Infinite
        }
    }

    // the qml example
    NanoShape {
        id: _shape
        x: 700
        y: 200
        width: 500
        height: 500

        property color strokeColor: "darkBlue"
        property real strokeWidth: 5

        onStrokeColorChanged: _shape.markDirty()
        onStrokeWidthChanged: _shape.markDirty()

        onPaint: {
            painter.moveTo(10, 10)
            painter.bezierTo(100, 200, 100, 300, _shape.width, _shape.height)
            painter.setStrokeWidth(_shape.strokeWidth)
            painter.setStrokeStyle(painter.dashPattern(_shape.strokeColor, [4, 1, 1, 1]))
            painter.stroke()
        }
    }
}
