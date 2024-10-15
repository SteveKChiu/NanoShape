//
// https://github.com/SteveKChiu/nanoshape
//
// Copyright 2024, Steve K. Chiu <steve.k.chiu@gmail.com>
//
// The MIT License (http://www.opensource.org/licenses/mit-license.php)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//

#include "NanoShapeExample.h"
#include "NanoPainter.h"

#include <QtMath>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define geometryChange geometryChanged
#endif

//---------------------------------------------------------------------------

NanoShapeExample::NanoShapeExample(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
    setAntialiasing(true);
}

NanoShapeExample::~NanoShapeExample()
{
    // do nothing
}

void NanoShapeExample::setEdges(int edges)
{
    if (m_edges == edges) return;
    m_edges = edges;
    m_dirtyNode = true;
    emit edgesChanged();
    update();
}

void NanoShapeExample::setStrokeWidth(qreal width)
{
    if (qFuzzyCompare(m_strokeWidth, width)) return;
    m_strokeWidth = width;
    m_dirtyNode = true;
    emit strokeWidthChanged();
    update();
}

void NanoShapeExample::setStrokeColor(const QColor& color)
{
    if (m_strokeColor == color) return;
    m_strokeColor = color;
    m_dirtyStroke = true;
    emit strokeColorChanged();
    update();
}

void NanoShapeExample::setFillColor(const QColor& color)
{
    if (m_fillColor == color) return;
    m_fillColor = color;
    m_dirtyFill = true;
    emit fillColorChanged();
    update();
}

void NanoShapeExample::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size()) m_dirtyNode = true;
}

QSGNode* NanoShapeExample::updatePaintNode(QSGNode* node, QQuickItem::UpdatePaintNodeData*)
{
    // do nothing if everything is fine
    if (node && !m_dirtyNode && !m_dirtyStroke && !m_dirtyFill) return node;

    // if only the brush is changed, we can change the material directly
    // without constructing the node again
    if (node && !m_dirtyNode) {
        bool ok = true;
        if (m_dirtyStroke) {
            m_dirtyStroke = false;
            ok &= NanoPainter::updatePaintNodeStrokeBrush(this, node, "shape", m_strokeColor);
        }

        if (m_dirtyFill) {
            m_dirtyFill = false;
            ok &= NanoPainter::updatePaintNodeFillBrush(this, node, "shape", m_fillColor);
        }

        // need to test if the change is ok, otherwise fallback to construct the node again
        if (ok) return node;
    }

    m_dirtyNode = false;
    m_dirtyStroke = false;
    m_dirtyFill = false;

    NanoPainter painter(this, node);
    painter.beginPath("shape");
    painter.setJoinStyle(Qt::RoundJoin);

    auto rx = width() / 2;
    auto ry = height() / 2;
    auto cx = rx;
    auto cy = ry;
    int edges = qMax(m_edges, 3);

    for (int i = 0; i < edges; i++) {
        auto d = qDegreesToRadians(360.0 * i / edges - 90.0);
        auto x = cx + rx * qCos(d);
        auto y = cy + ry * qSin(d);
        if (i == 0) {
            painter.moveTo(x, y);
        } else {
            painter.lineTo(x, y);
        }
    }
    painter.closeSubpath();

    if (m_fillColor.alpha() > 0) {
        painter.setFillBrush(m_fillColor);
        painter.fill();
    }

    if (m_strokeWidth > 0 && m_strokeColor.alpha() > 0) {
        painter.setStrokeWidth(m_strokeWidth);
        painter.setStrokeBrush(m_strokeColor);
        painter.stroke();
    }

    return painter.updatePaintNode();
}
