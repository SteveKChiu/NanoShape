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

#include "NanoShape.h"

#include <QQuickWindow>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define geometryChange geometryChanged
Q_DECLARE_METATYPE(NanoBrush)
#endif

//---------------------------------------------------------------------------

NanoShapePainter::NanoShapePainter(QQuickItem* item)
    : QObject(item)
    , NanoPainter(item)
{
    // do nothing
}

NanoShapePainter::~NanoShapePainter()
{
    // do nothing
}

QMatrix4x4 NanoShapePainter::transformMatrix() const
{
    return transform();
}

void NanoShapePainter::setTransformMatrix(const QMatrix4x4& matrix)
{
    setTransform(matrix.toTransform());
}

void NanoShapePainter::resetTransform()
{
    NanoPainter::resetTransform();
}

void NanoShapePainter::preTranslate(qreal x, qreal y)
{
    NanoPainter::preTranslate(x, y);
}

void NanoShapePainter::preRotate(qreal degree)
{
    NanoPainter::preRotate(degree);
}

void NanoShapePainter::preScale(qreal sx, qreal sy)
{
    NanoPainter::preScale(sx, sy);
}

void NanoShapePainter::preSkewX(qreal degree)
{
    NanoPainter::preSkewX(degree);
}

void NanoShapePainter::preSKewY(qreal degree)
{
    NanoPainter::preSKewY(degree);
}

void NanoShapePainter::preShear(qreal sh, qreal sv)
{
    NanoPainter::preShear(sh, sv);
}

void NanoShapePainter::postTranslate(qreal x, qreal y)
{
    NanoPainter::postTranslate(x, y);
}

void NanoShapePainter::postScale(qreal sx, qreal sy)
{
    NanoPainter::postScale(sx, sy);
}

void NanoShapePainter::postRotate(qreal degree)
{
    NanoPainter::postRotate(degree);
}

void NanoShapePainter::postSkewX(qreal degree)
{
    NanoPainter::postSkewX(degree);
}

void NanoShapePainter::postSKewY(qreal degree)
{
    NanoPainter::postSKewY(degree);
}

void NanoShapePainter::postShear(qreal sh, qreal sv)
{
    NanoPainter::postShear(sh, sv);
}

void NanoShapePainter::setCompositeStyle(int op)
{
    NanoPainter::setCompositeOperation(Composite(op));
}

void NanoShapePainter::setStrokeWidth(qreal width)
{
    NanoPainter::setStrokeWidth(width);
}

void NanoShapePainter::setDashOffset(qreal offset)
{
    NanoPainter::setDashOffset(offset);
}

void NanoShapePainter::setDashPattern(const QVector<qreal>& pattern)
{
    NanoPainter::setDashPattern(pattern);
}

void NanoShapePainter::setMiterLimit(qreal limit)
{
    NanoPainter::setMiterLimit(limit);
}

void NanoShapePainter::setCapStyle(Qt::PenCapStyle style)
{
    NanoPainter::setCapStyle(style);
}

void NanoShapePainter::setJoinStyle(Qt::PenJoinStyle style)
{
    NanoPainter::setJoinStyle(style);
}

static NanoBrush toNanoBrush(const QVariant& style)
{
    if (style.canConvert<NanoBrush>()) return style.value<NanoBrush>();
    if (style.canConvert<QColor>()) return style.value<QColor>();
    return QColor(style.toString());
}

void NanoShapePainter::setStrokeStyle(const QVariant& style)
{
    setStrokeBrush(toNanoBrush(style));
}

void NanoShapePainter::setFillStyle(const QVariant& style)
{
    setFillBrush(toNanoBrush(style));
}

QVariant NanoShapePainter::linearGradient(const QColor& startColor, const QColor& endColor,
        qreal sx, qreal sy, qreal ex, qreal ey)
{
    return QVariant::fromValue(NanoBrush::linearGradient(startColor, endColor, { sx, sy }, { ex, ey }));
}

QVariant NanoShapePainter::boxGradient(const QColor& innerColor, const QColor& outerColor,
        qreal bx, qreal by, qreal bw, qreal bh, qreal cornerRadius, qreal feather)
{
    return QVariant::fromValue(NanoBrush::boxGradient(innerColor, outerColor, { bx, by, bw, bh }, cornerRadius, feather));
}

QVariant NanoShapePainter::radialGradient(const QColor& innerColor, const QColor& outerColor,
        qreal cx, qreal cy, qreal innerRadius, qreal outerRadius)
{
    return QVariant::fromValue(NanoBrush::radialGradient(innerColor, outerColor, { cx, cy }, innerRadius, outerRadius));
}

QVariant NanoShapePainter::imagePattern(const QUrl& imageUrl, const QRectF& part, qreal rotation, qreal opacity)
{
    QString path;
    if (imageUrl.isLocalFile()) {
        path = imageUrl.toLocalFile();
    } else if (imageUrl.scheme() == "qrc") {
        path = imageUrl.path();
        if (path.startsWith('/')) {
            path = ':' + path;
        } else {
            path = QLatin1String(":/") + path;
        }
    } else {
        return QColor(Qt::black);
    }

    QImage image(path);
    if (image.isNull()) return QColor(Qt::black);
    return QVariant::fromValue(NanoBrush::imagePattern(image, part, rotation, opacity));
}

void NanoShapePainter::beginPath()
{
    NanoPainter::beginPath();
}

void NanoShapePainter::moveTo(float x, float y)
{
    NanoPainter::moveTo(x, y);
}

void NanoShapePainter::lineTo(float x, float y)
{
    NanoPainter::lineTo(x, y);
}

void NanoShapePainter::bezierTo(float c1x, float c1y, float c2x, float c2y, float x, float y)
{
    NanoPainter::bezierTo(c1x, c1y, c2x, c2y, x, y);
}

void NanoShapePainter::quadTo(float cx, float cy, float x, float y)
{
    NanoPainter::quadTo(cx, cy, x, y);
}

void NanoShapePainter::arcTo(float c1x, float c1y, float c2x, float c2y, float radius)
{
    NanoPainter::arcTo(c1x, c1y, c2x, c2y, radius);
}

void NanoShapePainter::closeSubpath()
{
    NanoPainter::closeSubpath();
}

void NanoShapePainter::addArc(float cx, float cy, float radius, float angle0, float angle1, bool clockwise)
{
    NanoPainter::addArc(cx, cy, radius, angle0, angle1, clockwise);
}

void NanoShapePainter::addRect(float x, float y, float width, float height)
{
    NanoPainter::addRect(x, y, width, height);
}

void NanoShapePainter::addRoundedRect(float x, float y, float width, float height, float radius)
{
    NanoPainter::addRoundedRect(x, y, width, height, radius);
}

void NanoShapePainter::addRoundedRect(float x, float y, float width, float height, float radiusTopLeft, float radiusTopRight, float radiusBottomLeft, float radiusBottomRight)
{
    NanoPainter::addRoundedRect(x, y, width, height, radiusTopLeft, radiusTopRight, radiusBottomLeft, radiusBottomRight);
}

void NanoShapePainter::addEllipse(float centerX, float centerY, float radiusX, float radiusY)
{
    NanoPainter::addEllipse(centerX, centerY, radiusX, radiusY);
}

void NanoShapePainter::addCircle(float centerX, float centerY, float radius)
{
    NanoPainter::addCircle(centerX, centerY, radius);
}

void NanoShapePainter::addPolygon(const QVariant& v)
{
    QPolygonF poly;
    if (v.canConvert<QPolygonF>()) {
        poly = v.value<QPolygonF>();
    } else if (v.canConvert<QVector<QPointF>>()) {
        poly = v.value<QVector<QPointF>>();
    } else {
        auto list = v.toList();
        if (!list.isEmpty()) {
            if (list.first().userType() == QMetaType::QPointF) {
                for (auto& item : list) {
                    poly += item.toPointF();
                }
            } else {
                for (int i = 0, n = list.count() - 1; i < n; i += 2) {
                    poly += QPointF(list.at(i).toDouble(), list.at(i + 1).toDouble());
                }
            }
        }
    }
    NanoPainter::addPolygon(poly);
}

void NanoShapePainter::asInverted()
{
    NanoPainter::asInverted();
}

void NanoShapePainter::stroke()
{
    NanoPainter::stroke();
}

void NanoShapePainter::fill()
{
    NanoPainter::fill();
}

//---------------------------------------------------------------------------

NanoShape::NanoShape(QQuickItem* parent)
    : QQuickItem(parent)
    , m_painter(this)
{
    setFlag(ItemHasContents);
    setAntialiasing(true);
}

NanoShape::~NanoShape()
{
    // do nothing
}

void NanoShape::markDirty()
{
    m_dirty = true;
    update();
}

void NanoShape::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size()) markDirty();
}

void NanoShape::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData& data)
{
    switch (change) {
    case ItemSceneChange:
        if (window()) {
            disconnect(window(), &QQuickWindow::afterAnimating, this, &NanoShape::prepare);
        }
        if (data.window) {
            connect(data.window, &QQuickWindow::afterAnimating, this, &NanoShape::prepare);
        }
        markDirty();
        break;
    case ItemAntialiasingHasChanged:
    case ItemDevicePixelRatioHasChanged:
        markDirty();
        break;
    default:
        break;
    }

    QQuickItem::itemChange(change, data);
}

void NanoShape::prepare()
{
    if (!m_dirty) return;
    m_painter.reset();
    emit paint(&m_painter);
}

QSGNode* NanoShape::updatePaintNode(QSGNode* node, QQuickItem::UpdatePaintNodeData*)
{
    if (!qFuzzyCompare(m_itemPixelRatio, m_painter.itemPixelRatio())) {
        m_itemPixelRatio = m_painter.itemPixelRatio();
        m_dirty = true;
    }

    if (node && !m_dirty) return node;
    m_dirty = false;
    return m_painter.updatePaintNode(node);
}
