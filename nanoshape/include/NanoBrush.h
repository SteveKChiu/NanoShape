//*
//* https://github.com/SteveKChiu/nanoshape
//*
//* Copyright 2020, Steve K. Chiu <steve.k.chiu@gmail.com>
//*
//* This Source Code Form is subject to the terms of the Mozilla Public
//* License, v. 2.0. If a copy of the MPL was not distributed with this
//* file, You can obtain one at https://mozilla.org/MPL/2.0/.
//*

#pragma once

#include <QColor>
#include <QImage>
#include <QRectF>

struct NVGpaint;

//---------------------------------------------------------------------------

class NanoBrush
{
public:
    NanoBrush();
    NanoBrush(Qt::GlobalColor color);
    NanoBrush(const QColor& color);
    NanoBrush(const NanoBrush& that) noexcept;
    NanoBrush(NanoBrush&& that) noexcept;
    ~NanoBrush();

    const QImage& image() const { return m_image; }
    const NVGpaint& paint() const { return *m_paint; }

    /// Creates a linear gradient.
    static NanoBrush linearGradient(const QColor& startColor, const QColor& endColor,
            const QPointF& start, const QPointF& end);

    /// Creates a box gradient. Box gradient is a feathered rounded rectangle, it is useful for rendering
    /// drop shadows or highlights for boxes.
    /// Parameter feather defines how blurry the border of the rectangle is.
    static NanoBrush boxGradient(const QColor& innerColor, const QColor& outerColor,
            const QRectF& box, qreal cornerRadius, qreal feather);

    /// Creates a radial gradient.
    static NanoBrush radialGradient(const QColor& innerColor, const QColor& outerColor,
            const QPointF& center, qreal innerRadius, qreal outerRadius);

    /// Creates and returns an image pattern.
    /// Parameters sourceRect specify the region of the image pattern,
    /// rotation (in degree) around the top-left corner
    static NanoBrush imagePattern(const QImage& image, const QRectF& sourceRect = {},
            qreal rotation = 0, qreal opacity = 1);

    /// pattern and offset will be multiple with unitWidth
    /// if unitWidth < 0, then it would be scaled with stroke width
    static NanoBrush dashPattern(const QColor& color, const QVector<qreal>& pattern, qreal offset = 0, qreal unitWidth = -1);

    bool operator==(const NanoBrush& that) const;
    bool operator!=(const NanoBrush& that) const { return !operator==(that); }

    NanoBrush& operator=(const NanoBrush& that) noexcept;
    NanoBrush& operator=(NanoBrush&& that) noexcept;

private:
    explicit NanoBrush(const NVGpaint& paint);

private:
    NVGpaint* m_paint;
    QImage m_image;
};
