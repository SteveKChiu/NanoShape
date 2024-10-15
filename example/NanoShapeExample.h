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

#include <QQuickItem>

//---------------------------------------------------------------------------

class NanoShapeExample : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int edges READ edges WRITE setEdges NOTIFY edgesChanged)
    Q_PROPERTY(qreal strokeWidth READ strokeWidth WRITE setStrokeWidth NOTIFY strokeWidthChanged)
    Q_PROPERTY(QColor strokeColor READ strokeColor WRITE setStrokeColor NOTIFY strokeColorChanged)
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)

public:
    explicit NanoShapeExample(QQuickItem* parent = nullptr);
    virtual ~NanoShapeExample();

    int edges() const { return m_edges; }
    void setEdges(int edges);

    qreal strokeWidth() const { return m_strokeWidth; }
    void setStrokeWidth(qreal width);

    QColor strokeColor() const { return m_strokeColor; }
    void setStrokeColor(const QColor& color);

    QColor fillColor() const { return m_fillColor; }
    void setFillColor(const QColor& color);

signals:
    void edgesChanged();
    void strokeWidthChanged();
    void strokeColorChanged();
    void fillColorChanged();

protected:
    virtual QSGNode* updatePaintNode(QSGNode* node, UpdatePaintNodeData*) override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;
#else
    virtual void geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry) override;
#endif

private:
    QColor m_fillColor = Qt::green;
    QColor m_strokeColor = Qt::blue;
    qreal m_strokeWidth = 4;
    int m_edges = 5;
    bool m_dirtyFill = true;
    bool m_dirtyStroke = true;
    bool m_dirtyNode = true;
};
