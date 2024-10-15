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
    /// rotation (in degree) around the top-left corner.
    static NanoBrush imagePattern(const QImage& image, const QRectF& sourceRect = {},
            qreal rotation = 0, qreal opacity = 1);

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
