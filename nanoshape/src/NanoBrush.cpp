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

#include "NanoBrush.h"
#include "nanovg.h"

//---------------------------------------------------------------------------

static inline NVGcolor toNVGcolor(const QColor& color)
{
    return nvgRGBAf(float(color.redF()), float(color.greenF()), float(color.blueF()), float(color.alphaF()));
}

NanoBrush::NanoBrush()
    : NanoBrush(QColor(Qt::transparent))
{
    // do nothing
}

NanoBrush::NanoBrush(Qt::GlobalColor color)
    : NanoBrush(QColor(color))
{
    // do nothing
}

NanoBrush::NanoBrush(const QColor& color)
    : m_paint(new NVGpaint)
{
    auto& p = *m_paint;
    memset(&p, 0, sizeof(p));
    nvgTransformIdentity(p.xform);
    p.radius = 0.0f;
    p.feather = 1.0f;
    p.innerColor = p.outerColor = toNVGcolor(color);
}

NanoBrush::NanoBrush(const NVGpaint& paint)
    : m_paint(new NVGpaint)
{
    *m_paint = paint;
}

NanoBrush::NanoBrush(const NanoBrush& that) noexcept
    : m_paint(new NVGpaint)
    , m_image(that.m_image)
{
    *m_paint = *that.m_paint;
}

NanoBrush::NanoBrush(NanoBrush&& that) noexcept
    : m_paint(that.m_paint)
    , m_image(that.m_image)
{
    that.m_paint = nullptr;
}

NanoBrush::~NanoBrush()
{
    delete m_paint;
}

NanoBrush NanoBrush::linearGradient(const QColor& startColor, const QColor& endColor, const QPointF& start, const QPointF& end)
{
    return NanoBrush(nvgLinearGradient(nullptr, float(start.x()), float(start.y()), float(end.x()), float(end.y()),
            toNVGcolor(startColor), toNVGcolor(endColor)));
}

NanoBrush NanoBrush::boxGradient(const QColor& innerColor, const QColor& outerColor, const QRectF& box, qreal cornerRadius, qreal feather)
{
    return NanoBrush(nvgBoxGradient(nullptr, float(box.x()), float(box.y()), float(box.width()), float(box.height()),
            float(cornerRadius), float(feather), toNVGcolor(innerColor), toNVGcolor(outerColor)));
}

NanoBrush NanoBrush::radialGradient(const QColor& innerColor, const QColor& outerColor, const QPointF& center, qreal innerRadius, qreal outerRadius)
{
    return NanoBrush(nvgRadialGradient(nullptr, float(center.x()), float(center.y()), float(innerRadius), float(outerRadius),
            toNVGcolor(innerColor), toNVGcolor(outerColor)));
}

NanoBrush NanoBrush::imagePattern(const QImage& image, const QRectF& rect, qreal rotation, qreal opacity)
{
    auto part = rect;
    if (part.isEmpty()) {
        part.setCoords(0, 0, image.width(), image.height());
    }

    NanoBrush brush(nvgImagePattern(nullptr, float(part.x()), float(part.y()), float(part.width()), float(part.height()),
            nvgDegToRad(float(rotation)), 1, float(opacity)));
    brush.m_image = image;
    return brush;
}

bool NanoBrush::operator==(const NanoBrush& that) const
{
    if (this == &that) return true;
    return m_image == that.m_image && memcmp(m_paint, that.m_paint, sizeof(*m_paint)) == 0;
}

NanoBrush& NanoBrush::operator=(const NanoBrush& that) noexcept
{
    if (this == &that) return *this;
    m_image = that.m_image;
    *m_paint = *that.m_paint;
    return *this;
}

NanoBrush& NanoBrush::operator=(NanoBrush&& that) noexcept
{
    qSwap(m_image, that.m_image);
    qSwap(m_paint, that.m_paint);
    return *this;
}
