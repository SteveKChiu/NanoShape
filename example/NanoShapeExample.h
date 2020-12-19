//*
//* Copyright 2020, Steve K. Chiu <steve.k.chiu@gmail.com>
//*
//* This Source Code Form is subject to the terms of the Mozilla Public
//* License, v. 2.0. If a copy of the MPL was not distributed with this
//* file, You can obtain one at https://mozilla.org/MPL/2.0/.
//*

#pragma once

#include <QQuickItem>

//---------------------------------------------------------------------------

class NanoShapeExample : public QQuickItem
{
    Q_OBJECT
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
