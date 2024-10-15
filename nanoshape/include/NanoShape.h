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

#include <QMatrix4x4>
#include <QQuickItem>

#include "NanoPainter.h"

//---------------------------------------------------------------------------

class NanoShapePainter : public QObject, public NanoPainter
{
    Q_OBJECT

public:
    explicit NanoShapePainter(QQuickItem* item);
    virtual ~NanoShapePainter();

    Q_INVOKABLE QMatrix4x4 transformMatrix() const;
    Q_INVOKABLE void setTransformMatrix(const QMatrix4x4& matrix);
    Q_INVOKABLE void resetTransform();

    Q_INVOKABLE void preTranslate(qreal x, qreal y);
    Q_INVOKABLE void preScale(qreal sx, qreal sy);
    Q_INVOKABLE void preRotate(qreal degree);
    Q_INVOKABLE void preSkewX(qreal degree);
    Q_INVOKABLE void preSKewY(qreal degree);
    Q_INVOKABLE void preShear(qreal sh, qreal sv);

    Q_INVOKABLE void postTranslate(qreal x, qreal y);
    Q_INVOKABLE void postScale(qreal sx, qreal sy);
    Q_INVOKABLE void postRotate(qreal degree);
    Q_INVOKABLE void postSkewX(qreal degree);
    Q_INVOKABLE void postSKewY(qreal degree);
    Q_INVOKABLE void postShear(qreal sh, qreal sv);

    // see NanoShape.CompositeStyle
    Q_INVOKABLE void setCompositeStyle(int op);

    Q_INVOKABLE void setCapStyle(Qt::PenCapStyle style);
    Q_INVOKABLE void setJoinStyle(Qt::PenJoinStyle style);
    Q_INVOKABLE void setMiterLimit(qreal limit);
    Q_INVOKABLE void setStrokeWidth(qreal width);

    // the dash offset and pattern will be scaled with stroke width
    Q_INVOKABLE void setDashOffset(qreal offset);
    Q_INVOKABLE void setDashPattern(const QVector<qreal>& pattern);

    // accept color, gradient or image pattern
    Q_INVOKABLE void setStrokeStyle(const QVariant& style);
    Q_INVOKABLE void setFillStyle(const QVariant& style);

    Q_INVOKABLE void beginPath();

    Q_INVOKABLE void moveTo(float x, float y);
    Q_INVOKABLE void lineTo(float x, float y);
    Q_INVOKABLE void bezierTo(float c1x, float c1y, float c2x, float c2y, float x, float y);
    Q_INVOKABLE void quadTo(float cx, float cy, float x, float y);
    Q_INVOKABLE void arcTo(float c1x, float c1y, float c2x, float c2y, float radius);
    Q_INVOKABLE void closeSubpath();

    Q_INVOKABLE void addArc(float cx, float cy, float radius, float angle0, float angle1, bool clockwise = true);
    Q_INVOKABLE void addRect(float x, float y, float width, float height);
    Q_INVOKABLE void addRoundedRect(float x, float y, float width, float height, float radius);
    Q_INVOKABLE void addRoundedRect(float x, float y, float width, float height, float radiusTopLeft, float radiusTopRight, float radiusBottomLeft, float radiusBottomRight);
    Q_INVOKABLE void addEllipse(float centerX, float centerY, float radiusX, float radiusY);
    Q_INVOKABLE void addCircle(float centerX, float centerY, float radius);

    // accept the following type:
    // QPolygonF, QVector<QPointF>, qml array of Qt.point, qml array of number (in [x0, y0, x1, y1, ...] format)
    Q_INVOKABLE void addPolygon(const QVariant& polygon);

    // make previous subpath as inverted (aka. hole)
    // subpath begins with moveTo, or other addXXX methods
    Q_INVOKABLE void asInverted();

    Q_INVOKABLE void stroke();
    Q_INVOKABLE void fill();

    Q_INVOKABLE static QVariant linearGradient(const QColor& startColor, const QColor& endColor,
            qreal sx, qreal sy, qreal ex, qreal ey);

    Q_INVOKABLE static QVariant boxGradient(const QColor& innerColor, const QColor& outerColor,
            qreal bx, qreal by, qreal bw, qreal bh, qreal cornerRadius, qreal feather);

    Q_INVOKABLE static QVariant radialGradient(const QColor& innerColor, const QColor& outerColor,
            qreal cx, qreal cy, qreal innerRadius, qreal outerRadius);

    Q_INVOKABLE static QVariant imagePattern(const QUrl& imageUrl,
            const QRectF& part = {}, qreal rotation = 0, qreal opacity = 1);
};

QML_DECLARE_TYPE(NanoShapePainter)

//---------------------------------------------------------------------------

class NanoShape : public QQuickItem
{
    Q_OBJECT

public:
    enum CompositeStyle
    {
        CompositeSourceOver = int(NanoPainter::Composite::SourceOver),
        CompositeSourceIn = int(NanoPainter::Composite::SourceIn),
        CompositeSourceOut = int(NanoPainter::Composite::SourceOut),
        CompositeAtop = int(NanoPainter::Composite::Atop),
        CompositeDestinationOver = int(NanoPainter::Composite::DestinationOver),
        CompositeDestinationIn = int(NanoPainter::Composite::DestinationIn),
        CompositeDestinationOut = int(NanoPainter::Composite::DestinationOut),
        CompositeDestinationAtop = int(NanoPainter::Composite::DestinationAtop),
        CompositeLighter = int(NanoPainter::Composite::Lighter),
        CompositeCopy = int(NanoPainter::Composite::Copy),
        CompositeXor = int(NanoPainter::Composite::Xor),
    };
    Q_ENUM(CompositeStyle)

public:
    explicit NanoShape(QQuickItem* parent = nullptr);
    virtual ~NanoShape();

    Q_INVOKABLE void markDirty();

signals:
    void paint(NanoShapePainter* painter);

protected:
    virtual void itemChange(ItemChange change, const ItemChangeData& data) override;
    virtual QSGNode* updatePaintNode(QSGNode* node, UpdatePaintNodeData*) override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;
#else
    virtual void geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry) override;
#endif

private:
    void prepare();

private:
    NanoShapePainter m_painter;
    float m_itemPixelRatio = 1;
    bool m_dirty = true;
};

QML_DECLARE_TYPE(NanoShape)
