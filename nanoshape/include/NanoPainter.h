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

#pragma once

#include "NanoBrush.h"

class QQuickItem;
class QSGNode;
class QSGGeometryNode;
class QSGGeometry;
class QSGMaterial;
class NanoPainterPrivate;

//---------------------------------------------------------------------------

class NanoPainter
{
public:
    enum class Composite
    {
        SourceOver,
        SourceIn,
        SourceOut,
        Atop,
        DestinationOver,
        DestinationIn,
        DestinationOut,
        DestinationAtop,
        Lighter,
        Copy,
        Xor,
    };

public:
    explicit NanoPainter(QQuickItem* item, float itemPixelRatio = 0);
    NanoPainter(QQuickItem* item, QSGNode* oldNode, float itemPixelRatio = 0);
    NanoPainter(NanoPainter&&) = delete;
    ~NanoPainter();

    float itemPixelRatio() const;

    void reset();
    void reset(QSGNode* oldNode);

    const QTransform& transform() const;
    void setTransform(const QTransform& transform);
    void resetTransform();

    void preTranslate(qreal x, qreal y);
    void preScale(qreal sx, qreal sy);
    void preRotate(qreal degree);
    void preSkewX(qreal degree);
    void preSKewY(qreal degree);
    void preShear(qreal sh, qreal sv);

    void postTranslate(qreal x, qreal y);
    void postScale(qreal sx, qreal sy);
    void postRotate(qreal degree);
    void postSkewX(qreal degree);
    void postSKewY(qreal degree);
    void postShear(qreal sh, qreal sv);

    Composite compositeOperation() const;
    void setCompositeOperation(Composite op);

    qreal miterLimit() const;
    void setMiterLimit(qreal limit);

    Qt::PenCapStyle capStyle() const;
    void setCapStyle(Qt::PenCapStyle style);

    Qt::PenJoinStyle joinStyle() const;
    void setJoinStyle(Qt::PenJoinStyle style);

    // the dash offset will be scaled with stroke width
    // just like QPen::dashOffset
    qreal dashOffset() const;
    void setDashOffset(qreal offset);

    // the dash pattern will be scaled with stroke width
    // just like QPen::dashPattern
    QVector<qreal> dashPattern() const;
    void setDashPattern(const QVector<qreal>& pattern);

    qreal strokeWidth() const;
    void setStrokeWidth(qreal width);

    const NanoBrush& strokeBrush() const;
    void setStrokeBrush(const NanoBrush& brush);

    const NanoBrush& fillBrush() const;
    void setFillBrush(const NanoBrush& brush);

    void beginPath(const QString& name = {});

    void moveTo(float x, float y);
    void moveTo(const QPointF& point);
    void lineTo(float x, float y);
    void lineTo(const QPointF& point);
    void bezierTo(float c1x, float c1y, float c2x, float c2y, float x, float y);
    void bezierTo(const QPointF& controlPoint1, const QPointF& controlPoint2, const QPointF& endPoint);
    void quadTo(float cx, float cy, float x, float y);
    void quadTo(const QPointF& controlPoint, const QPointF& endPoint);
    void arcTo(float c1x, float c1y, float c2x, float c2y, float radius);
    void arcTo(const QPointF& controlPoint1, const QPointF& controlPoint2, qreal radius);
    void closeSubpath();

    void addArc(float cx, float cy, float radius, float angle0, float angle1, bool clockwise = true);
    void addArc(const QPointF& centerPoint, qreal radius, qreal angle0, qreal angle1, bool clockwise = true);

    void addRect(float x, float y, float width, float height);
    void addRect(const QRectF& rect);

    void addRoundedRect(float x, float y, float width, float height, float radius);
    void addRoundedRect(const QRectF& rect, qreal radius);
    void addRoundedRect(float x, float y, float width, float height, float radiusTopLeft, float radiusTopRight, float radiusBottomLeft, float radiusBottomRight);
    void addRoundedRect(const QRectF& rect, qreal radiusTopLeft, qreal radiusTopRight, qreal radiusBottomLeft, qreal radiusBottomRight);

    void addEllipse(float centerX, float centerY, float radiusX, float radiusY);
    void addEllipse(const QRectF& rect);

    void addCircle(float centerX, float centerY, float radius);
    void addCircle(const QPointF& centerPoint, qreal radius);

    void addPolygon(const QPolygonF& polygon);
    void addPath(const QPainterPath& path);

    void asInverted();

    void stroke();
    void fill();

    QSGNode* updatePaintNode(QSGNode* node = nullptr);

    static float itemPixelRatio(QQuickItem* item);
    static bool updatePaintNodeStrokeBrush(QQuickItem* item, QSGNode* node, const QString& name, const NanoBrush& brush);
    static bool updatePaintNodeFillBrush(QQuickItem* item, QSGNode* node, const QString& name, const NanoBrush& brush);

private:
    NanoPainterPrivate* d;

    friend class NanoPainterPrivate;
};
