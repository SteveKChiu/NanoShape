//*
//* https://github.com/SteveKChiu/nanoshape
//*
//* Copyright 2020, Steve K. Chiu <steve.k.chiu@gmail.com>
//*
//* This Source Code Form is subject to the terms of the Mozilla Public
//* License, v. 2.0. If a copy of the MPL was not distributed with this
//* file, You can obtain one at https://mozilla.org/MPL/2.0/.
//*

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

void NanoShapePainter::postTranslate(qreal x, qreal y)
{
    NanoPainter::postTranslate(x, y);
}

void NanoShapePainter::postRotate(qreal degree)
{
    NanoPainter::postRotate(degree);
}

void NanoShapePainter::postShear(qreal sh, qreal sv)
{
    NanoPainter::postShear(sh, sv);
}

void NanoShapePainter::postScale(qreal sx, qreal sy)
{
    NanoPainter::postScale(sx, sy);
}

void NanoShapePainter::resetTransform()
{
    NanoPainter::resetTransform();
}

void NanoShapePainter::setCompositeStyle(int op)
{
    NanoPainter::setCompositeOperation(Composite(op));
}

void NanoShapePainter::setStrokeWidth(qreal width)
{
    NanoPainter::setStrokeWidth(width);
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

QVariant NanoShapePainter::dashPattern(const QColor& color, const QVector<qreal>& pattern,
        qreal offset, qreal unitWidth)
{
    return QVariant::fromValue(NanoBrush::dashPattern(color, pattern, offset, unitWidth));
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

void NanoShapePainter::closePath()
{
    NanoPainter::closePath();
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
        const auto list = v.toList();
        for (auto& item : list) {
            poly += item.toPointF();
        }
    }
    NanoPainter::addPath(poly);
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
    if (newGeometry.size() == oldGeometry.size()) return;
    markDirty();
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
    if (!m_dirty && node) return node;
    m_dirty = false;
    return m_painter.updatePaintNode(node);
}
